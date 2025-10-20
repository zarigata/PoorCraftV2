#pragma once

#include "poorcraft/modding/ModInfo.h"
#include "poorcraft/modding/ModAPI.h"
#include "poorcraft/platform/DynamicLibrary.h"

#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

namespace PoorCraft {

// Forward declare sol::protected_function
namespace sol { class protected_function; }

class LuaScriptEngine;

/**
 * @brief Loaded mod information
 */
struct LoadedMod {
    ModMetadata metadata;
    std::unique_ptr<DynamicLibrary> library;  // For native mods
    std::string scriptPath;                    // For Lua mods
    
    // Native plugin function pointers
    ModInfo* (*getModInfoFunc)();
    bool (*initFunc)(const ModAPI*);
    void (*updateFunc)(float);
    void (*shutdownFunc)();
    
    // Lua mod update function
    std::unique_ptr<sol::protected_function> luaUpdateFunc;
    
    // Event subscription tracking for cleanup
    std::vector<uint32_t> eventSubscriptions;
    
    std::chrono::time_point<std::chrono::system_clock> lastModifiedTime;
    bool enabled;

    LoadedMod()
        : getModInfoFunc(nullptr)
        , initFunc(nullptr)
        , updateFunc(nullptr)
        , shutdownFunc(nullptr)
        , luaUpdateFunc(nullptr)
        , enabled(true)
    {}
};

/**
 * @brief Mod manager singleton
 * 
 * Coordinates mod lifecycle: discovery, loading, updating, unloading, hot-reload.
 * Manages both native plugins and Lua scripts.
 */
class ModManager {
public:
    static ModManager& getInstance();

    /**
     * @brief Initialize mod manager
     * @param modsDirectory Path to mods directory
     */
    void initialize(const std::string& modsDirectory);
    
    /**
     * @brief Set engine system pointers for ModAPI
     */
    void setEntityManager(class EntityManager* entityManager);
    void setWorld(class World* world);
    void setChunkManager(class ChunkManager* chunkManager);

    /**
     * @brief Shutdown mod manager and unload all mods
     */
    void shutdown();

    /**
     * @brief Load all discovered mods
     */
    void loadMods();

    /**
     * @brief Unload specific mod
     * @param modId Mod ID
     */
    void unloadMod(const std::string& modId);

    /**
     * @brief Reload specific mod (hot-reload)
     * @param modId Mod ID
     */
    void reloadMod(const std::string& modId);

    /**
     * @brief Update all loaded mods
     * @param deltaTime Time since last update
     */
    void updateMods(float deltaTime);

    /**
     * @brief Check for file modifications and hot-reload
     */
    void checkForModifications();

    /**
     * @brief Get all loaded mods
     */
    const std::vector<LoadedMod>& getLoadedMods() const { return m_LoadedMods; }

    /**
     * @brief Check if mod is loaded
     */
    bool isModLoaded(const std::string& modId) const;

    /**
     * @brief Enable mod
     */
    void enableMod(const std::string& modId);

    /**
     * @brief Disable mod
     */
    void disableMod(const std::string& modId);

    // Non-copyable, non-movable
    ModManager(const ModManager&) = delete;
    ModManager& operator=(const ModManager&) = delete;
    ModManager(ModManager&&) = delete;
    ModManager& operator=(ModManager&&) = delete;

private:
    ModManager() = default;
    ~ModManager() = default;

    /**
     * @brief Discover mods in directory
     */
    void discoverMods();

    /**
     * @brief Resolve mod dependencies and sort load order
     */
    void resolveDependencies();

    /**
     * @brief Load native plugin mod
     */
    bool loadNativeMod(const ModMetadata& metadata);

    /**
     * @brief Load Lua script mod
     */
    bool loadLuaMod(const ModMetadata& metadata);

    /**
     * @brief Get file modification time
     */
    std::chrono::time_point<std::chrono::system_clock> getFileModTime(const std::string& path);

    std::string m_ModsDirectory;
    std::vector<ModMetadata> m_DiscoveredMods;
    std::vector<LoadedMod> m_LoadedMods;
    std::unique_ptr<LuaScriptEngine> m_LuaEngine;
    ModAPI m_ModAPI;
    
    // Engine system pointers for ModAPI
    class EntityManager* m_EntityManager;
    class World* m_World;
    class ChunkManager* m_ChunkManager;
};

} // namespace PoorCraft
