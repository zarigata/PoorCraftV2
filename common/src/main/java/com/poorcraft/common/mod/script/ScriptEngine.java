package com.poorcraft.common.mod.script;

import org.luaj.vm2.Globals;
import org.luaj.vm2.LuaError;
import org.luaj.vm2.LuaValue;
import org.luaj.vm2.Varargs;
import org.luaj.vm2.lib.jse.CoerceJavaToLua;
import org.luaj.vm2.lib.jse.CoerceLuaToJava;
import org.luaj.vm2.lib.jse.JsePlatform;

import java.io.Closeable;
import java.io.IOException;
import java.io.Reader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Objects;

public class ScriptEngine implements Closeable {
    private final boolean sandbox;
    private Globals globals;
    private boolean initialized;

    public ScriptEngine(boolean sandbox) {
        this.sandbox = sandbox;
    }

    public void init() {
        if (initialized) {
            return;
        }
        globals = sandbox ? createSandboxGlobals() : JsePlatform.standardGlobals();
        initialized = true;
    }

    public LuaValue loadScript(Path scriptPath) {
        ensureInitialized();
        Objects.requireNonNull(scriptPath, "scriptPath");
        try (Reader reader = Files.newBufferedReader(scriptPath, StandardCharsets.UTF_8)) {
            LuaValue chunk = globals.load(reader, scriptPath.toString());
            LuaValue result = chunk.call();
            return result.isnil() ? globals : result;
        } catch (IOException e) {
            throw new RuntimeException("Failed to load script: " + scriptPath, e);
        } catch (LuaError error) {
            throw new RuntimeException("Error executing script: " + scriptPath + " - " + error.getMessage(), error);
        }
    }

    public LuaValue executeFunction(LuaValue root, String functionName, Object... args) {
        ensureInitialized();
        Objects.requireNonNull(functionName, "functionName");
        LuaValue target = resolveFunction(root, functionName);
        if (target.isnil()) {
            return LuaValue.NIL;
        }
        LuaValue[] luaArgs = new LuaValue[args.length];
        for (int i = 0; i < args.length; i++) {
            luaArgs[i] = CoerceJavaToLua.coerce(args[i]);
        }
        try {
            Varargs result = target.invoke(LuaValue.varargsOf(luaArgs));
            return result.arg1();
        } catch (LuaError error) {
            throw new RuntimeException("Error executing Lua function '" + functionName + "': " + error.getMessage(), error);
        }
    }

    public void exposeAPI(String name, Object javaObject) {
        ensureInitialized();
        Objects.requireNonNull(name, "name");
        globals.set(name, CoerceJavaToLua.coerce(javaObject));
    }

    public Globals getGlobals() {
        ensureInitialized();
        return globals;
    }

    @Override
    public void close() throws IOException {
        globals = null;
        initialized = false;
    }

    private void ensureInitialized() {
        if (!initialized) {
            throw new IllegalStateException("ScriptEngine has not been initialized. Call init() first.");
        }
    }

    private Globals createSandboxGlobals() {
        Globals sandboxGlobals = JsePlatform.standardGlobals();
        sandboxGlobals.set("dofile", LuaValue.NIL);
        sandboxGlobals.set("loadfile", LuaValue.NIL);
        sandboxGlobals.set("load", LuaValue.NIL);
        sandboxGlobals.set("require", LuaValue.NIL);
        sandboxGlobals.set("package", LuaValue.NIL);
        sandboxGlobals.set("io", LuaValue.NIL);
        sandboxGlobals.set("os", LuaValue.tableOf());
        sandboxGlobals.set("debug", LuaValue.NIL);
        return sandboxGlobals;
    }

    private LuaValue resolveFunction(LuaValue root, String functionName) {
        if (root != null && !root.isnil()) {
            LuaValue fn = root.get(functionName);
            if (!fn.isnil()) {
                return fn;
            }
        }
        return globals.get(functionName);
    }
}
