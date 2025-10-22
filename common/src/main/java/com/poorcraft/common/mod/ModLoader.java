package com.poorcraft.common.mod;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.util.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class ModLoader {
    private static final org.slf4j.Logger LOGGER = Logger.getLogger(ModLoader.class);

    private final Path modsDirectory;
    private final EventBus eventBus;
    private final RegistryManager registryManager;
    private final Configuration configuration;
    private final Map<String, ModContainer> loadedMods = new HashMap<>();

    public ModLoader(Path modsDirectory, EventBus eventBus, RegistryManager registryManager, Configuration configuration) {
        this.modsDirectory = Objects.requireNonNull(modsDirectory, "modsDirectory");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.registryManager = Objects.requireNonNull(registryManager, "registryManager");
        this.configuration = Objects.requireNonNull(configuration, "configuration");
    }

    public List<ModMetadata> discoverMods() throws IOException {
        if (!Files.exists(modsDirectory)) {
            return Collections.emptyList();
        }
        List<ModMetadata> mods = new ArrayList<>();
        Files.walk(modsDirectory)
                .filter(path -> path.getFileName().toString().equalsIgnoreCase("mod.json"))
                .forEach(jsonPath -> {
                    try (InputStream stream = new FileInputStream(jsonPath.toFile())) {
                        ModMetadata metadata = ModMetadata.fromJson(stream);
                        metadata.validate();
                        metadata.setSourcePath(jsonPath.getParent());
                        mods.add(metadata);
                    } catch (Exception e) {
                        LOGGER.warn("Failed to load mod metadata from {}: {}", jsonPath, e.getMessage());
                    }
                });
        return mods;
    }

    public void loadMods() {
        try {
            List<ModMetadata> mods = discoverMods();
            loadMods(mods, configuration.getBoolean("mods.scriptSandbox", true));
        } catch (Exception e) {
            LOGGER.error("Failed to load mods: {}", e.getMessage(), e);
        }
    }

    public void loadMods(List<ModMetadata> mods, boolean sandbox) {
        try {
            List<ModMetadata> sorted = DependencyResolver.resolve(mods);
            for (ModMetadata metadata : sorted) {
                loadMod(metadata, sandbox);
            }
        } catch (Exception e) {
            LOGGER.error("Failed to load mods: {}", e.getMessage(), e);
        }
    }

    private void loadMod(ModMetadata metadata, boolean sandbox) throws IOException {
        if (loadedMods.containsKey(metadata.getId())) {
            return;
        }
        Path sourcePath = metadata.getSourcePath();
        if (sourcePath == null) {
            throw new IOException("Missing source path for mod '" + metadata.getId() + "'");
        }
        File jarFile = resolveJarFor(metadata, sourcePath);
        ModContainer container = new ModContainer(metadata, sourcePath, jarFile);
        container.load(eventBus, sandbox);
        loadedMods.put(metadata.getId(), container);
    }

    private File resolveJarFor(ModMetadata metadata, Path sourcePath) throws IOException {
        if (metadata.getMainClass() == null) {
            return null;
        }
        Path jarPath = sourcePath.resolveSibling(metadata.getId() + ".jar");
        if (!Files.exists(jarPath)) {
            jarPath = sourcePath.resolve(metadata.getId() + ".jar");
        }
        if (!Files.exists(jarPath)) {
            throw new IOException("Missing jar file for mod '" + metadata.getId() + "' near " + sourcePath);
        }
        return jarPath.toFile();
    }

    public void unloadMods() {
        loadedMods.values().forEach(container -> container.unload(eventBus));
        loadedMods.clear();
    }

    public ModContainer getLoadedMod(String id) {
        return loadedMods.get(id);
    }

    public List<ModContainer> getAllLoadedMods() {
        return List.copyOf(loadedMods.values());
    }
}
