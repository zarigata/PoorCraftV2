package com.poorcraft.client.entity;

import com.poorcraft.client.render.GLInfo;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.common.config.Configuration;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;
import org.lwjgl.system.MemoryUtil;
import org.slf4j.Logger;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;

public class SkinLoader {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(SkinLoader.class);
    private static final int[] SUPPORTED_DIMENSIONS = {32, 64};

    private final TextureManager textureManager;
    private final Configuration config;
    private final GLInfo capabilities;
    private final ExecutorService ioExecutor;
    private final boolean asyncEnabled;
    private final int cacheCapacity;
    private final float anisotropy;
    private final boolean srgb;
    private final Map<String, Texture2D> cache;
    private final Map<String, List<Consumer<Texture2D>>> pendingCallbacks;
    private final Deque<String> cacheOrder;
    private final ConcurrentLinkedQueue<Runnable> renderQueue;

    public SkinLoader(TextureManager textureManager, Configuration config) {
        this.textureManager = Objects.requireNonNull(textureManager, "textureManager");
        this.config = Objects.requireNonNull(config, "config");
        this.capabilities = textureManager.getCapabilities();
        this.asyncEnabled = config.getBoolean("entity.asyncSkinLoading", true);
        this.cacheCapacity = Math.max(1, config.getInt("entity.skinCacheSize", 100));
        this.anisotropy = config.getFloat("graphics.anisotropicFiltering", 1.0f);
        this.srgb = true;
        this.cache = new ConcurrentHashMap<>();
        this.pendingCallbacks = new ConcurrentHashMap<>();
        this.cacheOrder = new ArrayDeque<>();
        this.renderQueue = new ConcurrentLinkedQueue<>();
        this.ioExecutor = Executors.newFixedThreadPool(determineThreadCount(), new SkinThreadFactory());
    }

    public void loadSkin(String path, Consumer<Texture2D> callback) {
        Objects.requireNonNull(callback, "callback");
        String normalized = normalizePath(path);

        Texture2D cached = cache.get(normalized);
        if (cached != null) {
            enqueueRenderTask(() -> safelyInvoke(callback, cached));
            recordCacheAccess(normalized);
            return;
        }

        pendingCallbacks.compute(normalized, (key, callbacks) -> {
            if (callbacks == null) {
                List<Consumer<Texture2D>> list = new ArrayList<>();
                list.add(callback);
                submitLoadTask(normalized);
                return list;
            } else {
                callbacks.add(callback);
                return callbacks;
            }
        });
    }

    public void pumpRenderQueue() {
        Runnable task;
        while ((task = renderQueue.poll()) != null) {
            try {
                task.run();
            } catch (Exception e) {
                LOGGER.error("Skin render task failed", e);
            }
        }
    }

    public void cleanup() {
        renderQueue.clear();
        pendingCallbacks.clear();
        synchronized (cacheOrder) {
            for (String key : cacheOrder) {
                Texture2D texture = cache.remove(key);
                if (texture != null) {
                    try {
                        texture.close();
                    } catch (Exception e) {
                        LOGGER.warn("Failed to close skin texture '{}': {}", key, e.getMessage());
                    }
                }
            }
            cacheOrder.clear();
        }
        ioExecutor.shutdownNow();
    }

    private void submitLoadTask(String path) {
        Runnable task = () -> loadSkinData(path);
        if (asyncEnabled) {
            ioExecutor.submit(task);
        } else {
            task.run();
        }
    }

    private void loadSkinData(String path) {
        LoadedSkin loadedSkin = null;
        try {
            byte[] bytes = readSkinBytes(path);
            if (bytes == null || bytes.length == 0) {
                throw new IOException("Skin file empty: " + path);
            }
            loadedSkin = decodeSkin(bytes);
            if (!isSupportedDimensions(loadedSkin.width(), loadedSkin.height())) {
                throw new IOException("Unsupported skin dimensions: " + loadedSkin.width() + "x" + loadedSkin.height());
            }
            LoadedSkin finalSkin = loadedSkin;
            enqueueRenderTask(() -> handleUpload(path, finalSkin));
            loadedSkin = null;
        } catch (Exception e) {
            LOGGER.error("Failed to load skin '{}': {}", path, e.getMessage());
            enqueueRenderTask(() -> handleFailure(path));
        } finally {
            if (loadedSkin != null) {
                loadedSkin.release();
            }
        }
    }

    private void handleUpload(String path, LoadedSkin skin) {
        try {
            Texture2D texture = cache.get(path);
            if (texture == null) {
                texture = new Texture2D(skin.width(), skin.height(), skin.data(), capabilities, anisotropy, false, srgb, false);
                cache.put(path, texture);
                recordCacheAccess(path);
                enforceCacheLimit();
                LOGGER.debug("Uploaded skin '{}' ({}x{})", path, skin.width(), skin.height());
            }
            deliverCallbacks(path, texture);
        } catch (Exception e) {
            LOGGER.error("Failed to upload skin '{}': {}", path, e.getMessage());
            deliverCallbacks(path, textureManager.getMissingTexture());
        } finally {
            skin.release();
        }
    }

    private void handleFailure(String path) {
        deliverCallbacks(path, textureManager.getMissingTexture());
    }

    private void deliverCallbacks(String path, Texture2D texture) {
        List<Consumer<Texture2D>> callbacks = pendingCallbacks.remove(path);
        if (callbacks == null) {
            return;
        }
        recordCacheAccess(path);
        for (Consumer<Texture2D> callback : callbacks) {
            safelyInvoke(callback, texture);
        }
    }

    private void safelyInvoke(Consumer<Texture2D> callback, Texture2D texture) {
        try {
            callback.accept(texture);
        } catch (Exception e) {
            LOGGER.error("Skin callback threw", e);
        }
    }

    private void enqueueRenderTask(Runnable task) {
        renderQueue.add(task);
    }

    private void recordCacheAccess(String path) {
        if (path == null) {
            return;
        }
        synchronized (cacheOrder) {
            cacheOrder.remove(path);
            if (cache.containsKey(path)) {
                cacheOrder.addLast(path);
            }
        }
    }

    private void enforceCacheLimit() {
        synchronized (cacheOrder) {
            while (cache.size() > cacheCapacity && !cacheOrder.isEmpty()) {
                String evicted = cacheOrder.pollFirst();
                if (evicted == null) {
                    continue;
                }
                Texture2D removed = cache.remove(evicted);
                if (removed != null) {
                    try {
                        removed.close();
                    } catch (Exception e) {
                        LOGGER.warn("Failed to close evicted skin texture '{}': {}", evicted, e.getMessage());
                    }
                }
            }
        }
    }

    private String normalizePath(String path) {
        if (path == null || path.isBlank()) {
            return config.getString("entity.defaultPlayerSkin", "skins/player_default.png");
        }
        return path.trim();
    }

    private boolean isSupportedDimensions(int width, int height) {
        if (width != height) {
            return false;
        }
        for (int allowed : SUPPORTED_DIMENSIONS) {
            if (width == allowed) {
                return true;
            }
        }
        return false;
    }

    private byte[] readSkinBytes(String path) throws IOException {
        if (path.startsWith("http://") || path.startsWith("https://")) {
            try (InputStream stream = new URL(path).openStream()) {
                return stream.readAllBytes();
            }
        }

        Path filePath = Path.of(path);
        if (!Files.exists(filePath)) {
            Path base = Path.of(config.getString("resources.texturePath", "textures"));
            Path resolved = base.resolve(path).normalize();
            if (Files.exists(resolved)) {
                filePath = resolved;
            }
        }

        if (Files.exists(filePath)) {
            return Files.readAllBytes(filePath);
        }

        try (InputStream stream = getClass().getClassLoader().getResourceAsStream(path)) {
            if (stream != null) {
                return stream.readAllBytes();
            }
        }

        try (InputStream stream = getClass().getClassLoader().getResourceAsStream("textures/" + path)) {
            if (stream != null) {
                return stream.readAllBytes();
            }
        }

        throw new IOException("Skin not found: " + path);
    }

    private LoadedSkin decodeSkin(byte[] bytes) throws IOException {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer widthBuf = stack.mallocInt(1);
            IntBuffer heightBuf = stack.mallocInt(1);
            IntBuffer channelsBuf = stack.mallocInt(1);
            ByteBuffer data = STBImage.stbi_load_from_memory(ByteBuffer.wrap(bytes), widthBuf, heightBuf, channelsBuf, 4);
            if (data != null) {
                return new LoadedSkin(data, widthBuf.get(0), heightBuf.get(0), () -> STBImage.stbi_image_free(data));
            }
        }

        BufferedImage image = ImageIO.read(new ByteArrayInputStream(bytes));
        if (image == null) {
            throw new IOException("Failed to decode skin image");
        }

        int width = image.getWidth();
        int height = image.getHeight();
        ByteBuffer buffer = MemoryUtil.memAlloc(width * height * 4);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int argb = image.getRGB(x, y);
                buffer.put((byte) ((argb >> 16) & 0xFF));
                buffer.put((byte) ((argb >> 8) & 0xFF));
                buffer.put((byte) (argb & 0xFF));
                buffer.put((byte) ((argb >> 24) & 0xFF));
            }
        }
        buffer.flip();
        return new LoadedSkin(buffer, width, height, () -> MemoryUtil.memFree(buffer));
    }

    private int determineThreadCount() {
        int available = Math.max(1, Runtime.getRuntime().availableProcessors() / 2);
        return Math.max(1, Math.min(available, 4));
    }

    private static final class LoadedSkin {
        private final ByteBuffer data;
        private final int width;
        private final int height;
        private final Runnable releaser;

        LoadedSkin(ByteBuffer data, int width, int height, Runnable releaser) {
            this.data = data;
            this.width = width;
            this.height = height;
            this.releaser = releaser;
        }

        ByteBuffer data() {
            return data;
        }

        int width() {
            return width;
        }

        int height() {
            return height;
        }

        void release() {
            releaser.run();
        }
    }

    private static final class SkinThreadFactory implements ThreadFactory {
        private final AtomicInteger counter = new AtomicInteger();

        @Override
        public Thread newThread(Runnable r) {
            Thread thread = new Thread(r, "SkinLoader-" + counter.incrementAndGet());
            thread.setDaemon(true);
            return thread;
        }
    }
}
