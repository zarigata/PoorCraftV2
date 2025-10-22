package com.poorcraft.client.mod;

import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.mod.ModLoader;
import com.poorcraft.common.mod.ModMetadata;
import com.poorcraft.common.network.packet.ModDataPacket;
import com.poorcraft.common.network.packet.ModListPacket;
import com.poorcraft.common.network.packet.ModRequestPacket;
import com.poorcraft.common.registry.RegistryManager;
import org.slf4j.Logger;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ClientModManager {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ClientModManager.class);

    private final Configuration configuration;
    private final EventBus eventBus;
    private final RegistryManager registryManager;
    private final ClientNetworkManager networkManager;
    private final Path modsDirectory;
    private final ModLoader modLoader;
    private final Map<String, ModListPacket.ModInfo> requiredMods = new ConcurrentHashMap<>();
    private final Set<String> completedMods = ConcurrentHashMap.newKeySet();
    private final Map<String, byte[][]> chunkBuffers = new ConcurrentHashMap<>();
    private final Map<String, Integer> expectedChunks = new ConcurrentHashMap<>();
    private final Map<String, Set<Integer>> receivedChunks = new ConcurrentHashMap<>();
    private final Map<String, Long> receivedBytes = new ConcurrentHashMap<>();
    private final boolean autoDownload;
    private final long maxModSize;

    public ClientModManager(Configuration configuration,
                             EventBus eventBus,
                             RegistryManager registryManager,
                             ClientNetworkManager networkManager) {
        this.configuration = Objects.requireNonNull(configuration, "configuration");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.registryManager = Objects.requireNonNull(registryManager, "registryManager");
        this.networkManager = networkManager;
        this.modsDirectory = Path.of(configuration.getString("mods.directory", "mods"));
        this.autoDownload = configuration.getBoolean("mods.autoDownload", false);
        this.maxModSize = configuration.getLong("mods.maxModSize", 50L * 1024L * 1024L);
        this.modLoader = new ModLoader(modsDirectory, eventBus, registryManager, configuration);
    }

    public void init() throws IOException {
        Files.createDirectories(modsDirectory);
        modLoader.loadMods();
        markLoadedMods();
    }

    public void handleModList(ModListPacket packet) {
        requiredMods.clear();
        completedMods.clear();
        packet.getMods().forEach(info -> requiredMods.put(info.getId(), info));
        markLoadedMods();
        requestMissingMods();
        maybeFinalize();
    }

    public void handleModData(ModDataPacket packet) {
        String modId = packet.getModId();
        if (!requiredMods.containsKey(modId)) {
            LOGGER.warn("Received mod data for unknown mod {}", modId);
            return;
        }
        if (!autoDownload) {
            LOGGER.info("Ignoring mod data for {} because auto-download is disabled", modId);
            return;
        }
        int totalChunks = packet.getTotalChunks();
        if (totalChunks <= 0) {
            LOGGER.warn("Invalid totalChunks {} for mod {}", totalChunks, modId);
            return;
        }
        expectedChunks.putIfAbsent(modId, totalChunks);
        if (expectedChunks.get(modId) != totalChunks) {
            LOGGER.warn("Conflicting total chunk count for mod {} ({} vs {})", modId, expectedChunks.get(modId), totalChunks);
            resetModDownload(modId);
            requestMod(modId);
            return;
        }
        if (packet.getChunkIndex() < 0 || packet.getChunkIndex() >= totalChunks) {
            LOGGER.warn("Invalid chunk index {} for mod {}", packet.getChunkIndex(), modId);
            return;
        }
        receivedChunks.computeIfAbsent(modId, ignored -> ConcurrentHashMap.newKeySet());
        if (!receivedChunks.get(modId).add(packet.getChunkIndex())) {
            LOGGER.debug("Duplicate chunk {} for mod {} ignored", packet.getChunkIndex(), modId);
            return;
        }
        byte[][] buffers = chunkBuffers.computeIfAbsent(modId, ignored -> new byte[totalChunks][]);
        if (buffers.length != totalChunks) {
            LOGGER.warn("Conflicting buffer size for mod {} ({} vs {})", modId, buffers.length, totalChunks);
            resetModDownload(modId);
            requestMod(modId);
            return;
        }
        if (buffers[packet.getChunkIndex()] != null) {
            LOGGER.debug("Chunk {} for mod {} already stored", packet.getChunkIndex(), modId);
            return;
        }
        byte[] chunkCopy = Arrays.copyOf(packet.getData(), packet.getData().length);
        buffers[packet.getChunkIndex()] = chunkCopy;
        long totalBytes = receivedBytes.getOrDefault(modId, 0L) + chunkCopy.length;
        receivedBytes.put(modId, totalBytes);
        if (totalBytes > maxModSize) {
            LOGGER.error("Mod {} exceeded max size {} bytes", modId, maxModSize);
            resetModDownload(modId);
            requestMod(modId);
            return;
        }
        if (receivedChunks.get(modId).size() == totalChunks) {
            finalizeModData(modId, buffers, packet.getChecksum());
        }
    }

    private void finalizeModData(String modId, byte[][] chunks, String packetChecksum) {
        try {
            ByteArrayOutputStream assembled = new ByteArrayOutputStream();
            for (int i = 0; i < chunks.length; i++) {
                byte[] chunk = chunks[i];
                if (chunk == null) {
                    LOGGER.warn("Missing chunk {} for mod {} during finalization", i, modId);
                    resetModDownload(modId);
                    requestMod(modId);
                    return;
                }
                assembled.write(chunk, 0, chunk.length);
            }
            byte[] data = assembled.toByteArray();
            String computedChecksum = computeChecksum(data);
            String expectedChecksum = packetChecksum;
            if (expectedChecksum == null) {
                ModListPacket.ModInfo info = requiredMods.get(modId);
                if (info != null) {
                    expectedChecksum = info.getChecksum();
                }
            }
            if (expectedChecksum != null && !expectedChecksum.isBlank() && !expectedChecksum.equals(computedChecksum)) {
                LOGGER.error("Checksum mismatch for mod {} (expected {}, got {})", modId, expectedChecksum, computedChecksum);
                resetModDownload(modId);
                requestMod(modId);
                return;
            }
            Path target = modsDirectory.resolve(modId + ".jar");
            Files.write(target, data);
            LOGGER.info("Downloaded mod {} ({} bytes)", modId, data.length);
            resetModDownload(modId);
            completedMods.add(modId);
            ModListPacket.ModInfo info = requiredMods.get(modId);
            if (info != null && !checksumMatches(target, info.getChecksum())) {
                LOGGER.warn("Local mod {} checksum mismatch after download", modId);
            }
            maybeFinalize();
        } catch (IOException e) {
            LOGGER.error("Failed to persist mod {}", modId, e);
            resetModDownload(modId);
            requestMod(modId);
        }
    }

    private void markLoadedMods() {
        for (ModMetadata metadata : getLocalMods()) {
            String modId = metadata.getId();
            Path jarPath = modsDirectory.resolve(modId + ".jar");
            if (!Files.exists(jarPath)) {
                continue;
            }
            ModListPacket.ModInfo info = requiredMods.get(modId);
            if (info == null) {
                continue;
            }
            try {
                if (checksumMatches(jarPath, info.getChecksum())) {
                    completedMods.add(modId);
                }
            } catch (IOException e) {
                LOGGER.warn("Failed to verify checksum for {}", modId, e);
            }
        }
    }

    private void requestMissingMods() {
        if (!autoDownload || networkManager == null) {
            return;
        }
        for (Map.Entry<String, ModListPacket.ModInfo> entry : requiredMods.entrySet()) {
            String modId = entry.getKey();
            if (completedMods.contains(modId)) {
                continue;
            }
            requestMod(modId);
        }
    }

    private void requestMod(String modId) {
        if (networkManager == null) {
            return;
        }
        ClientConnection connection = networkManager.getConnection();
        if (connection == null) {
            LOGGER.warn("Cannot request mod {} without connection", modId);
            return;
        }
        LOGGER.info("Requesting mod {} from server", modId);
        networkManager.sendPacket(new ModRequestPacket(modId));
    }

    private void maybeFinalize() {
        if (requiredMods.isEmpty()) {
            tryFinalizeLoad();
            return;
        }
        if (completedMods.containsAll(requiredMods.keySet())) {
            tryFinalizeLoad();
        }
    }

    private void tryFinalizeLoad() {
        try {
            modLoader.unloadMods();
            modLoader.loadMods();
            registryManager.freezeAll();
            LOGGER.info("Client mods loaded and registries frozen");
        } catch (Exception e) {
            LOGGER.error("Failed to finalize mod loading", e);
        }
    }

    private void resetModDownload(String modId) {
        chunkBuffers.remove(modId);
        expectedChunks.remove(modId);
        receivedChunks.remove(modId);
        completedMods.remove(modId);
        receivedBytes.remove(modId);
    }

    private String computeChecksum(byte[] bytes) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(bytes);
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalStateException("SHA-256 not available", e);
        }
    }

    private boolean checksumMatches(Path path, String expected) throws IOException {
        if (expected == null || expected.isBlank()) {
            return false;
        }
        byte[] bytes = Files.readAllBytes(path);
        String computed = computeChecksum(bytes);
        return expected.equals(computed);
    }

    private List<ModMetadata> getLocalMods() {
        try {
            return modLoader.discoverMods();
        } catch (IOException e) {
            LOGGER.error("Failed to discover local mods", e);
            return List.of();
        }
    }

    public boolean isAutoDownloadEnabled() {
        return autoDownload;
    }
}
