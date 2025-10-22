package com.poorcraft.server.mod;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.mod.ModLoader;
import com.poorcraft.common.mod.ModMetadata;
import com.poorcraft.common.mod.ModMetadata.ModDependency;
import com.poorcraft.common.mod.ModContainer;
import com.poorcraft.common.mod.DependencyResolver.DependencyException;
import com.poorcraft.common.network.packet.ModListPacket;
import com.poorcraft.common.registry.RegistryManager;
import com.poorcraft.common.util.Logger;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Base64;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ServerModManager {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ServerModManager.class);

    private final Configuration configuration;
    private final EventBus eventBus;
    private final RegistryManager registryManager;
    private final Path modsDirectory;
    private final ModLoader modLoader;
    private final Map<String, byte[]> modDataCache = new ConcurrentHashMap<>();
    private final Map<String, String> checksumCache = new ConcurrentHashMap<>();

    public ServerModManager(Configuration configuration, EventBus eventBus, RegistryManager registryManager) {
        this.configuration = Objects.requireNonNull(configuration, "configuration");
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.registryManager = Objects.requireNonNull(registryManager, "registryManager");
        this.modsDirectory = Path.of(configuration.getString("mods.directory", "mods"));
        this.modLoader = new ModLoader(modsDirectory, eventBus, registryManager, configuration);
    }

    public void init() throws IOException {
        Files.createDirectories(modsDirectory);
        loadApprovedMods();
        cacheModData();
    }

    public void shutdown() {
        modLoader.unloadMods();
        modDataCache.clear();
        checksumCache.clear();
    }

    private void loadApprovedMods() throws IOException {
        List<String> allowed = configuration.getStringList("mods.allowedPermissions");
        Set<String> allowedPermissions = new HashSet<>(allowed);
        long maxModSize = configuration.getLong("mods.maxModSize", 50L * 1024L * 1024L);
        boolean sandbox = configuration.getBoolean("mods.scriptSandbox", true);

        List<ModMetadata> discovered = modLoader.discoverMods();
        List<ModMetadata> approved = new ArrayList<>();

        for (ModMetadata metadata : discovered) {
            Set<String> requested = metadata.getPermissions();
            if (!allowedPermissions.containsAll(requested)) {
                Set<String> disallowed = new HashSet<>(requested);
                disallowed.removeAll(allowedPermissions);
                LOGGER.warn("Skipping mod {} due to disallowed permissions {}", metadata.getId(), disallowed);
                continue;
            }

            Path jarPath = metadata.getSourcePath().resolveSibling(metadata.getId() + ".jar");
            if (!Files.exists(jarPath)) {
                jarPath = metadata.getSourcePath().resolve(metadata.getId() + ".jar");
            }
            if (Files.exists(jarPath)) {
                long size = Files.size(jarPath);
                if (size > maxModSize) {
                    LOGGER.warn("Skipping mod {} because size {} exceeds limit {}", metadata.getId(), size, maxModSize);
                    continue;
                }
            }

            approved.add(metadata);
        }

        modLoader.loadMods(approved, sandbox);
    }

    public List<ModListPacket.ModInfo> getModList() {
        List<ModListPacket.ModInfo> mods = new ArrayList<>();
        for (ModContainer container : modLoader.getAllLoadedMods()) {
            ModMetadata metadata = container.getMetadata();
            String id = metadata.getId();
            String version = metadata.getVersion();
            byte[] data = modDataCache.get(id);
            String checksum = checksumCache.getOrDefault(id, "");
            long size = data != null ? data.length : 0;
            mods.add(new ModListPacket.ModInfo(id, version, size, checksum));
        }
        return mods;
    }

    public byte[] getModData(String modId) {
        return modDataCache.get(modId);
    }

    public String getChecksum(String modId) {
        return checksumCache.get(modId);
    }

    public String ensureChecksum(String modId) {
        byte[] data = modDataCache.get(modId);
        if (data == null) {
            return null;
        }
        return checksumCache.computeIfAbsent(modId, key -> computeChecksum(data));
    }

    private void cacheModData() {
        for (ModContainer container : modLoader.getAllLoadedMods()) {
            ModMetadata metadata = container.getMetadata();
            if (metadata.getMainClass() == null) {
                continue;
            }
            try {
                Path jarPath = metadata.getSourcePath().resolveSibling(metadata.getId() + ".jar");
                if (!Files.exists(jarPath)) {
                    jarPath = metadata.getSourcePath().resolve(metadata.getId() + ".jar");
                }
                if (!Files.exists(jarPath)) {
                    LOGGER.warn("Cannot find jar file for mod {}", metadata.getId());
                    continue;
                }
                byte[] bytes = Files.readAllBytes(jarPath);
                modDataCache.put(metadata.getId(), bytes);
                checksumCache.put(metadata.getId(), computeChecksum(bytes));
            } catch (IOException e) {
                LOGGER.error("Failed to cache mod data for {}", metadata.getId(), e);
            }
        }
    }

    private String computeChecksum(byte[] data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(data);
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalStateException("SHA-256 not available", e);
        }
    }
}
