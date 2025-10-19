#pragma once

#include <memory>
#include <string>
#include <vector>

// Forward declare sol types to avoid including sol.hpp in header
namespace sol {
    class state;
}

namespace PoorCraft {

/**
 * @brief Lua scripting engine wrapper using sol2
 * 
 * Manages Lua VM and engine bindings for mod scripts.
 * Provides type-safe C++ to Lua bindings via sol2.
 */
class LuaScriptEngine {
public:
    LuaScriptEngine();
    ~LuaScriptEngine();

    // Non-copyable, non-movable
    LuaScriptEngine(const LuaScriptEngine&) = delete;
    LuaScriptEngine& operator=(const LuaScriptEngine&) = delete;
    LuaScriptEngine(LuaScriptEngine&&) = delete;
    LuaScriptEngine& operator=(LuaScriptEngine&&) = delete;

    /**
     * @brief Initialize Lua state and register engine bindings
     */
    void initialize();

    /**
     * @brief Shutdown Lua state
     */
    void shutdown();

    /**
     * @brief Execute Lua script from file
     * @param scriptPath Path to .lua file
     * @return true on success
     */
    bool executeScript(const std::string& scriptPath);

    /**
     * @brief Execute Lua code string
     * @param luaCode Lua code to execute
     * @return true on success
     */
    bool executeString(const std::string& luaCode);

    /**
     * @brief Call global Lua function
     * @param functionName Function name
     * @param args Arguments to pass
     * @return true on success
     */
    template<typename... Args>
    bool callFunction(const std::string& functionName, Args&&... args);

    /**
     * @brief Register C++ function as Lua global
     * @param name Function name in Lua
     * @param function C++ function
     */
    template<typename Func>
    void registerGlobalFunction(const std::string& name, Func&& function);

    /**
     * @brief Get global variable value
     * @tparam T Value type
     * @param name Variable name
     * @return Value or default
     */
    template<typename T>
    T getGlobal(const std::string& name, T defaultValue = T{});

    /**
     * @brief Set global variable value
     * @tparam T Value type
     * @param name Variable name
     * @param value Value to set
     */
    template<typename T>
    void setGlobal(const std::string& name, const T& value);

    /**
     * @brief Get Lua state (for advanced usage)
     */
    sol::state* getState() { return m_State.get(); }

private:
    /**
     * @brief Register all engine bindings
     */
    void registerEngineBindings();

    /**
     * @brief Handle Lua error
     */
    void handleLuaError(const std::string& error);

    std::unique_ptr<sol::state> m_State;
    std::vector<std::string> m_ScriptPaths;  // For hot-reload tracking
};

} // namespace PoorCraft
