package com.poorcraft.client.ui;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.network.handler.ChatMessageHandler;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.ui.screens.*;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.client.window.Window;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.packet.ChatMessagePacket;
import com.poorcraft.core.Updatable;

import java.util.EnumMap;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

/**
 * Manages UI state and screen lifecycle for the retained-mode UI system.
 */
public class UIManager implements Updatable {
    private final Window window;
    private final InputManager inputManager;
    private final ClientNetworkManager networkManager;
    private final TextureManager textureManager;
    private final Configuration config;

    private UIState currentState;
    private final Map<UIState, UIScreen> screens;
    private final List<UIStateListener> listeners;

    public UIManager(Window window, InputManager inputManager, ClientNetworkManager networkManager, TextureManager textureManager, Configuration config) {
        this.window = window;
        this.inputManager = inputManager;
        this.networkManager = networkManager;
        this.textureManager = textureManager;
        this.config = config;
        this.screens = new EnumMap<>(UIState.class);
        this.listeners = new CopyOnWriteArrayList<>();
        this.currentState = UIState.MAIN_MENU;
    }

    /**
     * Initialize the UI manager by creating and registering all screens.
     */
    public void init() {
        // Initialize ItemSlot texture manager
        com.poorcraft.client.ui.widgets.ItemSlot.setTextureManager(textureManager);
        
        // Create and register all UI screens
        screens.put(UIState.MAIN_MENU, new MainMenuScreen(this, config));
        screens.put(UIState.SERVER_BROWSER, new ServerBrowserScreen(this, config, networkManager));
        screens.put(UIState.SETTINGS, new SettingsScreen(this, config));
        screens.put(UIState.INVENTORY, new InventoryScreen(this, config, textureManager, networkManager));
        screens.put(UIState.IN_GAME, new HUDScreen(this, config, textureManager));

        ChatOverlay chatOverlay = new ChatOverlay(this, config);
        if (networkManager != null) {
            chatOverlay.setNetworkManager(networkManager);
            networkManager.registerHandler(ChatMessagePacket.class, new ChatMessageHandler(networkManager, chatOverlay));
        }
        screens.put(UIState.CHAT, chatOverlay);
        screens.put(UIState.PAUSED, new PauseScreen(this, config));
        screens.put(UIState.CONNECTING, new ConnectionScreen(this, config));

        // Initialize current screen
        UIScreen currentScreen = screens.get(currentState);
        if (currentScreen != null) {
            currentScreen.onShow();
            currentScreen.resize(window.getWidth(), window.getHeight());
        }
    }
    
    /**
     * Set the player entity for HUD and Inventory screens.
     * Should be called when the player spawns or changes.
     */
    public void setPlayerEntity(com.poorcraft.common.entity.Entity playerEntity) {
        UIScreen hudScreen = screens.get(UIState.IN_GAME);
        if (hudScreen instanceof HUDScreen) {
            ((HUDScreen) hudScreen).setPlayerEntity(playerEntity);
        }
        
        UIScreen inventoryScreen = screens.get(UIState.INVENTORY);
        if (inventoryScreen instanceof InventoryScreen) {
            ((InventoryScreen) inventoryScreen).setPlayerEntity(playerEntity);
        }
    }

    /**
     * Set the current UI state and manage screen lifecycle.
     */
    public void setState(UIState newState) {
        if (newState == currentState) {
            return;
        }

        // Hide current screen
        UIScreen currentScreen = screens.get(currentState);
        if (currentScreen != null) {
            currentScreen.onHide();
        }

        UIState previousState = currentState;
        currentState = newState;

        // Show new screen
        UIScreen newScreen = screens.get(newState);
        if (newScreen != null) {
            newScreen.onShow();
            newScreen.resize(window.getWidth(), window.getHeight());
        }

        // Notify listeners
        for (UIStateListener listener : listeners) {
            listener.onStateChanged(previousState, newState);
        }
    }

    /**
     * Get the current UI state.
     */
    public UIState getCurrentState() {
        return currentState;
    }

    /**
     * Get the screen for a specific state.
     */
    public UIScreen getScreen(UIState state) {
        return screens.get(state);
    }

    /**
     * Update the current screen.
     */
    @Override
    public void update(double dt) {
        UIScreen currentScreen = screens.get(currentState);
        if (currentScreen != null) {
            currentScreen.update((float) dt);
        }
    }

    /**
     * Render the current screen.
     */
    public void render(UIRenderer renderer) {
        UIScreen currentScreen = screens.get(currentState);
        if (currentScreen != null) {
            currentScreen.render(renderer);
        }
    }

    /**
     * Handle input for the current screen.
     * @return true if the UI consumed the input, false otherwise
     */
    public boolean handleInput(InputManager input) {
        UIScreen currentScreen = screens.get(currentState);
        if (currentScreen != null) {
            return currentScreen.handleInput(input);
        }
        return false;
    }

    /**
     * Handle window resize by updating all screens.
     */
    public void resize(int width, int height) {
        for (UIScreen screen : screens.values()) {
            screen.resize(width, height);
        }
    }

    public Window getWindow() {
        return window;
    }

    public int getViewportWidth() {
        return window.getWidth();
    }

    public int getViewportHeight() {
        return window.getHeight();
    }

    public void setCursorMode(int mode) {
        window.setCursorMode(mode);
    }

    /**
     * Add a state change listener.
     */
    public void addStateListener(UIStateListener listener) {
        if (listener != null) {
            listeners.add(listener);
        }
    }

    /**
     * Remove a state change listener.
     */
    public void removeStateListener(UIStateListener listener) {
        listeners.remove(listener);
    }

    /**
     * Clean up all screens.
     */
    public void cleanup() {
        for (UIScreen screen : screens.values()) {
            screen.cleanup();
        }
        screens.clear();
        listeners.clear();
    }

    /**
     * Interface for listening to UI state changes.
     */
    public interface UIStateListener {
        void onStateChanged(UIState previousState, UIState newState);
    }
}
