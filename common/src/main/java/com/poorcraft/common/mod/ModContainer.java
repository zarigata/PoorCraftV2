package com.poorcraft.common.mod;

import com.poorcraft.api.Mod;
import com.poorcraft.common.event.EventBus;
import com.poorcraft.common.mod.script.ScriptEngine;
import com.poorcraft.common.mod.script.ScriptGameAPI;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.nio.file.Path;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import org.luaj.vm2.LuaValue;

public class ModContainer implements Closeable {
    public enum ModState {
        UNLOADED,
        LOADING,
        LOADED,
        ERROR
    }

    private final ModMetadata metadata;
    private final Path modDirectory;
    private final File jarFile;
    private URLClassLoader classLoader;
    private Object modInstance;
    private ModState state;
    private String errorMessage;
    private final List<Object> eventListeners = new ArrayList<>();
    private ScriptEngine scriptEngine;
    private LuaValue scriptRoot;

    public ModContainer(ModMetadata metadata, Path modDirectory, File jarFile) {
        this.metadata = Objects.requireNonNull(metadata, "metadata");
        this.modDirectory = Objects.requireNonNull(modDirectory, "modDirectory");
        this.jarFile = jarFile;
        this.state = ModState.UNLOADED;
    }

    public String getId() {
        return metadata.getId();
    }

    public ModMetadata getMetadata() {
        return metadata;
    }

    public Object getModInstance() {
        return modInstance;
    }

    public ModState getState() {
        return state;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    public URLClassLoader getClassLoader() {
        return classLoader;
    }

    public void load(EventBus eventBus, boolean sandboxScripts) {
        Objects.requireNonNull(eventBus, "eventBus");
        if (state == ModState.LOADED) {
            return;
        }
        state = ModState.LOADING;
        try {
            if (metadata.getMainClass() != null || jarFile != null) {
                this.classLoader = createClassLoader();
                this.modInstance = instantiateMod();
                registerEventListeners(eventBus);
                invokeLifecycleMethod("init");
            }

            if (metadata.getScriptFile() != null) {
                loadScriptMod(sandboxScripts);
            }
            state = ModState.LOADED;
        } catch (Exception e) {
            state = ModState.ERROR;
            errorMessage = e.getMessage();
            throw new RuntimeException("Failed to load mod '" + metadata.getId() + "'", e);
        }
    }

    public void unload(EventBus eventBus) {
        if (state != ModState.LOADED && state != ModState.ERROR) {
            return;
        }
        try {
            invokeLifecycleMethod("shutdown");
        } catch (Exception e) {
            // ignore shutdown errors
        }
        unregisterEventListeners(eventBus);
        unloadScript();
        try {
            close();
        } catch (IOException ignored) {
        }
        state = ModState.UNLOADED;
        modInstance = null;
        classLoader = null;
    }

    @Override
    public void close() throws IOException {
        if (classLoader != null) {
            classLoader.close();
        }
    }

    private URLClassLoader createClassLoader() throws MalformedURLException {
        if (jarFile == null) {
            return new URLClassLoader(new URL[0], getClass().getClassLoader());
        }
        URL url = jarFile.toURI().toURL();
        return new URLClassLoader(new URL[]{url}, getClass().getClassLoader());
    }

    private Object instantiateMod() throws ClassNotFoundException, NoSuchMethodException, InvocationTargetException,
            InstantiationException, IllegalAccessException {
        String mainClass = metadata.getMainClass();
        if (mainClass == null) {
            mainClass = findAnnotatedModClass();
            if (mainClass == null) {
                return null;
            }
        }
        Class<?> clazz = Class.forName(mainClass, true, classLoader);
        if (!clazz.isAnnotationPresent(Mod.class)) {
            throw new IllegalStateException("Main class '" + mainClass + "' for mod '" + metadata.getId() + "' is not annotated with @Mod");
        }
        return clazz.getDeclaredConstructor().newInstance();
    }

    private void invokeLifecycleMethod(String methodName) {
        if (modInstance == null) {
            return;
        }
        try {
            Method method = modInstance.getClass().getMethod(methodName);
            method.invoke(modInstance);
        } catch (NoSuchMethodException ignored) {
            // lifecycle method optional
        } catch (InvocationTargetException | IllegalAccessException e) {
            throw new RuntimeException("Failed to invoke lifecycle method '" + methodName + "' for mod '" + metadata.getId() + "'", e);
        }
    }

    private void registerEventListeners(EventBus eventBus) {
        Object listener = modInstance;
        if (listener != null) {
            eventBus.register(listener);
            eventListeners.add(listener);
        }
    }

    private void unregisterEventListeners(EventBus eventBus) {
        eventListeners.forEach(eventBus::unregister);
        eventListeners.clear();
    }

    private void loadScriptMod(boolean sandbox) {
        Path scriptPath = modDirectory.resolve(metadata.getScriptFile());
        scriptEngine = new ScriptEngine(sandbox);
        scriptEngine.init();
        scriptRoot = scriptEngine.loadScript(scriptPath);
        scriptEngine.exposeAPI("game", new ScriptGameAPI(metadata.getId()));
        scriptEngine.executeFunction(scriptRoot, "init");
    }

    private void unloadScript() {
        if (scriptEngine == null) {
            return;
        }
        try {
            scriptEngine.executeFunction(scriptRoot, "shutdown");
        } catch (RuntimeException ignored) {
            // ignore shutdown failures for scripts
        }
        try {
            scriptEngine.close();
        } catch (IOException ignored) {
        } finally {
            scriptEngine = null;
            scriptRoot = null;
        }
    }

    private String findAnnotatedModClass() {
        if (jarFile == null) {
            return null;
        }
        try (JarFile jar = new JarFile(jarFile)) {
            for (JarEntry entry : java.util.Collections.list(jar.entries())) {
                if (entry.isDirectory() || !entry.getName().endsWith(".class")) {
                    continue;
                }
                String className = entry.getName()
                    .replace('/', '.')
                    .replaceAll("\\.class$", "");
                Class<?> clazz;
                try {
                    clazz = Class.forName(className, false, classLoader);
                } catch (ClassNotFoundException e) {
                    continue;
                }
                if (clazz.isAnnotationPresent(Mod.class) && !clazz.isInterface() && !Modifier.isAbstract(clazz.getModifiers())) {
                    return className;
                }
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to scan mod jar for @Mod classes", e);
        }
        return null;
    }
}
