#include "poorcraft/modding/ModManager.h"
#include "poorcraft/modding/LuaScriptEngine.h"
#include "poorcraft/modding/ModEvents.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/world/World.h"
#include "poorcraft/world/ChunkManager.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace PoorCraft {

ModManager& ModManager::getInstance() {
    static ModManager instance;
    return instance;
}

void ModManager::setEntityManager(EntityManager* entityManager) {
    m_EntityManager = entityManager;
    // Recreate ModAPI with updated pointers
    m_ModAPI = createModAPI(m_EntityManager, m_World, m_ChunkManager);
    // Update LuaScriptEngine pointers
    if (m_LuaEngine) {
        m_LuaEngine->setEntityManager(m_EntityManager);
    }
}

void ModManager::setWorld(World* world) {
    m_World = world;
    // Also set ChunkManager from World if available
    if (m_World) {
        m_ChunkManager = &m_World->getChunkManager();
    }
    // Recreate ModAPI with updated pointers
    m_ModAPI = createModAPI(m_EntityManager, m_World, m_ChunkManager);
    // Update LuaScriptEngine pointers
    if (m_LuaEngine) {
        m_LuaEngine->setWorld(m_World);
        m_LuaEngine->setChunkManager(m_ChunkManager);
    }
}

void ModManager::setChunkManager(ChunkManager* chunkManager) {
    m_ChunkManager = chunkManager;
    // Recreate ModAPI with updated pointers
    m_ModAPI = createModAPI(m_EntityManager, m_World, m_ChunkManager);
    // Update LuaScriptEngine pointers
    if (m_LuaEngine) {
        m_LuaEngine->setChunkManager(m_ChunkManager);
    }
}

void ModManager::initialize(const std::string& modsDirectory) {
    PC_INFO("Initializing ModManager...");
    
    m_ModsDirectory = modsDirectory;
    
    // Initialize engine system pointers to nullptr
    m_EntityManager = nullptr;
    m_World = nullptr;
    m_ChunkManager = nullptr;
    
    // Create Lua engine
    m_LuaEngine = std::make_unique<LuaScriptEngine>();
    m_LuaEngine->initialize();
    
    // Create ModAPI (with null pointers initially)
    m_ModAPI = createModAPI(m_EntityManager, m_World, m_ChunkManager);
    
    // Discover mods
    discoverMods();
    
    // Resolve dependencies
    resolveDependencies();
    
    PC_INFO("ModManager initialized, found {} mods", m_DiscoveredMods.size());
}

void ModManager::shutdown() {
    PC_INFO("Shutting down ModManager...");
    
    // Unload all mods in reverse order
    for (auto it = m_LoadedMods.rbegin(); it != m_LoadedMods.rend(); ++it) {
        if (it->enabled) {
            // Call shutdown function
            if (it->metadata.isNative && it->shutdownFunc) {
                it->shutdownFunc();
            }
            
            // Publish unload event
            ModUnloadedEvent event(it->metadata.id, "Shutdown");
            EventBus::getInstance().publish(event);
            
            PC_INFO("Unloaded mod: {}", it->metadata.name);
        }
    }
    
    m_LoadedMods.clear();
    m_DiscoveredMods.clear();
    
    // Shutdown Lua engine
    if (m_LuaEngine) {
        m_LuaEngine->shutdown();
        m_LuaEngine.reset();
    }
    
    PC_INFO("ModManager shut down");
}

void ModManager::discoverMods() {
    PC_INFO("Discovering mods in: {}", m_ModsDirectory);
    
    // Check if mods directory exists
    if (!Platform::directory_exists(m_ModsDirectory)) {
        PC_WARN("Mods directory does not exist: {}", m_ModsDirectory);
        return;
    }
    
    // List subdirectories
    std::vector<Platform::DirectoryEntry> entries;
    Platform::list_directory(m_ModsDirectory, entries, false);
    
    for (const auto& entry : entries) {
        if (entry.type != Platform::FileType::Directory) {
            continue;
        }
        
        // Check for mod.json
        std::string manifestPath = Platform::join_path(m_ModsDirectory, entry.name);
        manifestPath = Platform::join_path(manifestPath, "mod.json");
        
        if (!Platform::file_exists(manifestPath)) {
            continue;
        }
        
        try {
            // Parse manifest
            ModMetadata metadata = ModManifest::parseManifest(manifestPath);
            
            // Validate metadata
            if (!ModManifest::validateMetadata(metadata)) {
                PC_ERROR("Invalid mod metadata: {}", metadata.id);
                continue;
            }
            
            // Check if enabled in config
            std::string enableKey = "Mods." + metadata.id + ".enabled";
            metadata.enabled = poorcraft::Config::get_instance().get_bool(enableKey, true);
            
            m_DiscoveredMods.push_back(metadata);
            PC_INFO("Discovered mod: {} v{} ({})", metadata.name, metadata.version, 
                    metadata.enabled ? "enabled" : "disabled");
            
        } catch (const std::exception& e) {
            PC_ERROR("Failed to parse mod manifest {}: {}", manifestPath, e.what());
        }
    }
    
    PC_INFO("Discovered {} mods", m_DiscoveredMods.size());
}

void ModManager::resolveDependencies() {
    if (m_DiscoveredMods.empty()) {
        return;
    }
    
    PC_DEBUG("Resolving mod dependencies...");
    
    // Build dependency graph
    std::unordered_map<std::string, std::vector<std::string>> graph;
    std::unordered_map<std::string, int> inDegree;
    std::unordered_map<std::string, ModMetadata*> modMap;
    
    for (auto& mod : m_DiscoveredMods) {
        modMap[mod.id] = &mod;
        inDegree[mod.id] = 0;
        graph[mod.id] = {};
    }
    
    // Build edges (dependency -> dependent)
    for (auto& mod : m_DiscoveredMods) {
        for (const auto& dep : mod.dependencies) {
            if (modMap.find(dep) == modMap.end()) {
                PC_WARN("Mod {} depends on missing mod: {}", mod.id, dep);
                continue;
            }
            graph[dep].push_back(mod.id);
            inDegree[mod.id]++;
        }
    }
    
    // Topological sort using Kahn's algorithm
    std::queue<std::string> queue;
    for (const auto& [id, degree] : inDegree) {
        if (degree == 0) {
            queue.push(id);
        }
    }
    
    std::vector<ModMetadata> sorted;
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        sorted.push_back(*modMap[current]);
        
        for (const auto& dependent : graph[current]) {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0) {
                queue.push(dependent);
            }
        }
    }
    
    // Check for circular dependencies
    if (sorted.size() != m_DiscoveredMods.size()) {
        PC_ERROR("Circular dependency detected in mods!");
        // Keep only mods without circular dependencies
        m_DiscoveredMods = sorted;
    } else {
        m_DiscoveredMods = sorted;
    }
    
    // Sort by load priority within dependency levels
    std::stable_sort(m_DiscoveredMods.begin(), m_DiscoveredMods.end(),
        [](const ModMetadata& a, const ModMetadata& b) {
            return a.loadPriority < b.loadPriority;
        });
    
    PC_DEBUG("Dependency resolution complete");
}

void ModManager::loadMods() {
    PC_INFO("Loading mods...");
    
    int loadedCount = 0;
    for (const auto& metadata : m_DiscoveredMods) {
        if (!metadata.enabled) {
            PC_INFO("Skipping disabled mod: {}", metadata.name);
            continue;
        }
        
        bool success = false;
        if (metadata.isNative) {
            success = loadNativeMod(metadata);
        } else {
            success = loadLuaMod(metadata);
        }
        
        if (success) {
            loadedCount++;
            
            // Publish mod loaded event
            ModLoadedEvent event(metadata.id, metadata.name, metadata.version);
            EventBus::getInstance().publish(event);
        }
    }
    
    PC_INFO("Loaded {} mods", loadedCount);
}

bool ModManager::loadNativeMod(const ModMetadata& metadata) {
    PC_INFO("Loading native mod: {}", metadata.name);
    
    try {
        LoadedMod loadedMod;
        loadedMod.metadata = metadata;
        loadedMod.enabled = true;
        
        // Load library
        loadedMod.library = std::make_unique<DynamicLibrary>(metadata.libraryPath);
        
        // Resolve entry points
        loadedMod.getModInfoFunc = loadedMod.library->getSymbol<ModInfo*(*)()>("GetModInfo");
        loadedMod.initFunc = loadedMod.library->getSymbol<bool(*)(const ModAPI*)>("InitializeMod");
        loadedMod.updateFunc = loadedMod.library->tryGetSymbol<void(*)(float)>("UpdateMod");
        loadedMod.shutdownFunc = loadedMod.library->getSymbol<void(*)()>("ShutdownMod");
        
        // Get mod info
        ModInfo* modInfo = loadedMod.getModInfoFunc();
        if (!modInfo) {
            PC_ERROR("GetModInfo returned null for mod: {}", metadata.id);
            return false;
        }
        
        // Validate API version
        if (modInfo->apiVersion != ENGINE_API_VERSION) {
            PC_ERROR("API version mismatch for mod {}: expected {}, got {}", 
                     metadata.id, ENGINE_API_VERSION, modInfo->apiVersion);
            return false;
        }
        
        // Set mod context for subscription tracking
        ModAPI_SetCurrentModContext(&loadedMod.eventSubscriptions);
        
        // Initialize mod
        if (!loadedMod.initFunc(&m_ModAPI)) {
            PC_ERROR("InitializeMod failed for mod: {}", metadata.id);
            ModAPI_SetCurrentModContext(nullptr);
            return false;
        }
        
        // Clear mod context
        ModAPI_SetCurrentModContext(nullptr);
        
        // Get file modification time
        loadedMod.lastModifiedTime = getFileModTime(metadata.libraryPath);
        
        m_LoadedMods.push_back(std::move(loadedMod));
        PC_INFO("Successfully loaded native mod: {}", metadata.name);
        return true;
        
    } catch (const std::exception& e) {
        PC_ERROR("Failed to load native mod {}: {}", metadata.id, e.what());
        return false;
    }
}

bool ModManager::loadLuaMod(const ModMetadata& metadata) {
    PC_INFO("Loading Lua mod: {}", metadata.name);
    
    try {
        if (!m_LuaEngine) {
            PC_ERROR("Lua engine not initialized");
            return false;
        }
        
        // Execute script
        if (!m_LuaEngine->executeScript(metadata.libraryPath)) {
            PC_ERROR("Failed to execute Lua script: {}", metadata.libraryPath);
            return false;
        }
        
        LoadedMod loadedMod;
        loadedMod.metadata = metadata;
        loadedMod.scriptPath = metadata.libraryPath;
        loadedMod.enabled = true;
        loadedMod.lastModifiedTime = getFileModTime(metadata.libraryPath);
        
        // Store per-mod update function if it exists
        sol::state* luaState = m_LuaEngine->getState();
        if (luaState) {
            sol::optional<sol::protected_function> updateFunc = (*luaState)["update"];
            if (updateFunc) {
                loadedMod.luaUpdateFunc = std::make_unique<sol::protected_function>(*updateFunc);
                PC_DEBUG("Lua mod '{}' has update function", metadata.name);
                // Clear global update to avoid conflicts with next mod
                (*luaState)["update"] = sol::nil;
            }
        }
        
        m_LoadedMods.push_back(std::move(loadedMod));
        PC_INFO("Successfully loaded Lua mod: {}", metadata.name);
        return true;
        
    } catch (const std::exception& e) {
        PC_ERROR("Failed to load Lua mod {}: {}", metadata.id, e.what());
        return false;
    }
}

void ModManager::unloadMod(const std::string& modId) {
    auto it = std::find_if(m_LoadedMods.begin(), m_LoadedMods.end(),
        [&modId](const LoadedMod& mod) { return mod.metadata.id == modId; });
    
    if (it == m_LoadedMods.end()) {
        PC_WARN("Mod not loaded: {}", modId);
        return;
    }
    
    PC_INFO("Unloading mod: {}", it->metadata.name);
    
    // Unsubscribe all event subscriptions for this mod
    for (uint32_t modSubId : it->eventSubscriptions) {
        m_ModAPI.unsubscribeEvent(modSubId);
        PC_DEBUG("Unsubscribed event {} for mod {}", modSubId, modId);
    }
    it->eventSubscriptions.clear();
    
    // Call shutdown function
    if (it->metadata.isNative && it->shutdownFunc) {
        it->shutdownFunc();
    }
    
    // Publish unload event
    ModUnloadedEvent event(modId, "Manual unload");
    EventBus::getInstance().publish(event);
    
    m_LoadedMods.erase(it);
    PC_INFO("Mod unloaded: {}", modId);
}

void ModManager::reloadMod(const std::string& modId) {
    PC_INFO("Reloading mod: {}", modId);
    
    // Find mod metadata
    auto metaIt = std::find_if(m_DiscoveredMods.begin(), m_DiscoveredMods.end(),
        [&modId](const ModMetadata& mod) { return mod.id == modId; });
    
    if (metaIt == m_DiscoveredMods.end()) {
        PC_ERROR("Mod metadata not found: {}", modId);
        return;
    }
    
    // Unload if currently loaded
    unloadMod(modId);
    
    // Reload
    bool success = false;
    if (metaIt->isNative) {
        success = loadNativeMod(*metaIt);
    } else {
        success = loadLuaMod(*metaIt);
    }
    
    if (success) {
        ModReloadedEvent event(modId);
        EventBus::getInstance().publish(event);
        PC_INFO("Mod reloaded: {}", modId);
    } else {
        PC_ERROR("Failed to reload mod: {}", modId);
    }
}

void ModManager::updateMods(float deltaTime) {
    for (auto& mod : m_LoadedMods) {
        if (!mod.enabled) {
            continue;
        }
        
        try {
            if (mod.metadata.isNative && mod.updateFunc) {
                // Set mod context to track subscriptions created during UpdateMod
                ModAPI_SetCurrentModContext(&mod.eventSubscriptions);
                mod.updateFunc(deltaTime);
                ModAPI_SetCurrentModContext(nullptr);
            } else if (!mod.metadata.isNative && mod.luaUpdateFunc) {
                // Call per-mod Lua update function
                sol::protected_function_result result = (*mod.luaUpdateFunc)(deltaTime);
                if (!result.valid()) {
                    sol::error err = result;
                    PC_ERROR("Lua mod '{}' update failed: {}", mod.metadata.id, err.what());
                }
            }
        } catch (const std::exception& e) {
            PC_ERROR("Error updating mod {}: {}", mod.metadata.id, e.what());
        }
    }
}

void ModManager::checkForModifications() {
    for (auto& mod : m_LoadedMods) {
        if (!mod.enabled) {
            continue;
        }
        
        std::string path = mod.metadata.isNative ? mod.metadata.libraryPath : mod.scriptPath;
        auto currentModTime = getFileModTime(path);
        
        if (currentModTime > mod.lastModifiedTime) {
            PC_INFO("Detected modification in mod: {}, hot-reloading...", mod.metadata.name);
            reloadMod(mod.metadata.id);
        }
    }
}

bool ModManager::isModLoaded(const std::string& modId) const {
    return std::any_of(m_LoadedMods.begin(), m_LoadedMods.end(),
        [&modId](const LoadedMod& mod) { return mod.metadata.id == modId && mod.enabled; });
}

void ModManager::enableMod(const std::string& modId) {
    // Update config
    std::string enableKey = "Mods." + modId + ".enabled";
    poorcraft::Config::get_instance().set_bool(enableKey, true);
    
    // Load if not already loaded
    if (!isModLoaded(modId)) {
        auto it = std::find_if(m_DiscoveredMods.begin(), m_DiscoveredMods.end(),
            [&modId](const ModMetadata& mod) { return mod.id == modId; });
        
        if (it != m_DiscoveredMods.end()) {
            if (it->isNative) {
                loadNativeMod(*it);
            } else {
                loadLuaMod(*it);
            }
        }
    }
}

void ModManager::disableMod(const std::string& modId) {
    // Update config
    std::string enableKey = "Mods." + modId + ".enabled";
    poorcraft::Config::get_instance().set_bool(enableKey, false);
    
    // Unload if currently loaded
    if (isModLoaded(modId)) {
        unloadMod(modId);
    }
}

std::chrono::time_point<std::chrono::system_clock> ModManager::getFileModTime(const std::string& path) {
    try {
        int64_t modTime = Platform::get_file_modification_time(path);
        return std::chrono::system_clock::from_time_t(static_cast<time_t>(modTime));
    } catch (...) {
        return std::chrono::system_clock::now();
    }
}

} // namespace PoorCraft
