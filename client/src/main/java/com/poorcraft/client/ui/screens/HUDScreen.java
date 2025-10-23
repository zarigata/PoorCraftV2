package com.poorcraft.client.ui.screens;

import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.ItemSlot;
import com.poorcraft.client.ui.widgets.Label;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.common.Constants;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.HealthComponent;
import com.poorcraft.common.entity.component.InventoryComponent;
import com.poorcraft.common.inventory.ItemStack;

import java.io.IOException;

/**
 * HUD screen overlay for in-game information with textured UI elements.
 */
public class HUDScreen extends UIScreen {
    private final TextureManager textureManager;
    private Entity playerEntity;
    
    // UI Textures
    private Texture2D crosshairTexture;
    private Texture2D heartFullTexture;
    private Texture2D heartHalfTexture;
    private Texture2D heartEmptyTexture;
    private Texture2D hungerFullTexture;
    private Texture2D hungerHalfTexture;
    private Texture2D hungerEmptyTexture;
    private Texture2D hotbarTexture;
    private Texture2D hotbarSelectionTexture;
    
    // UI Elements
    private Panel rootPanel;
    private ItemSlot[] hotbarSlots;
    private Label debugLabel;
    private boolean showDebug;

    public HUDScreen(UIManager uiManager, Configuration config, TextureManager textureManager) {
        super(uiManager, config);
        this.textureManager = textureManager;
        this.showDebug = config.getBoolean("ui.showDebug", false);
    }
    
    public void setPlayerEntity(Entity playerEntity) {
        this.playerEntity = playerEntity;
    }

    @Override
    public void onShow() {
        super.onShow();
        loadUITextures();
    }
    
    private void loadUITextures() {
        try {
            crosshairTexture = loadUITexture("crosshair.png");
            heartFullTexture = loadUITexture("heart_full.png");
            heartHalfTexture = loadUITexture("heart_half.png");
            heartEmptyTexture = loadUITexture("heart_empty.png");
            hungerFullTexture = loadUITexture("hunger_full.png");
            hungerHalfTexture = loadUITexture("hunger_half.png");
            hungerEmptyTexture = loadUITexture("hunger_empty.png");
            hotbarTexture = loadUITexture("hotbar.png");
            hotbarSelectionTexture = loadUITexture("hotbar_selection.png");
        } catch (IOException e) {
            System.err.println("Failed to load UI textures: " + e.getMessage());
        }
    }
    
    private Texture2D loadUITexture(String filename) throws IOException {
        return textureManager.loadTexture("ui/" + filename);
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        // HUD is full-screen transparent overlay
        rootPanel = new Panel(0, 0, screenWidth, screenHeight);
        rootPanel.setBackgroundColor(Color.TRANSPARENT);

        // Create hotbar slots
        hotbarSlots = new ItemSlot[Constants.Inventory.HOTBAR_SIZE];
        for (int i = 0; i < Constants.Inventory.HOTBAR_SIZE; i++) {
            hotbarSlots[i] = new ItemSlot(0, 0, Constants.UI.HOTBAR_SLOT_SIZE);
            hotbarSlots[i].setItemStack(ItemStack.EMPTY);
        }

        // Debug info top left (optional)
        if (showDebug) {
            debugLabel = new Label(10, 10, 360, 60, "XYZ: 0,0,0\nFPS: 0\nBiome: Plains");
            debugLabel.setColor(Color.WHITE);
            rootPanel.addChild(debugLabel);
        }

        setRootWidget(rootPanel);
    }
    
    @Override
    public void update(float dt) {
        super.update(dt);
        
        // Update hotbar slots from player inventory
        if (playerEntity != null) {
            InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
            if (inventory != null) {
                for (int i = 0; i < Constants.Inventory.HOTBAR_SIZE; i++) {
                    ItemStack stack = inventory.getInventory().getHotbarSlot(i);
                    hotbarSlots[i].setItemStack(stack);
                }
            }
        }
    }
    
    @Override
    public void render(UIRenderer renderer) {
        if (rootPanel == null) {
            return;
        }
        
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();
        
        // Render crosshair centered
        if (crosshairTexture != null) {
            float crosshairSize = Constants.UI.CROSSHAIR_SIZE;
            float crosshairX = (screenWidth / 2f) - (crosshairSize / 2f);
            float crosshairY = (screenHeight / 2f) - (crosshairSize / 2f);
            renderer.drawTexturedRect(crosshairX, crosshairY, crosshairSize, crosshairSize, crosshairTexture);
        }
        
        // Render health hearts bottom-left
        if (playerEntity != null) {
            HealthComponent health = playerEntity.getComponent(HealthComponent.class);
            if (health != null) {
                renderHearts(renderer, health, 10, screenHeight - 30);
                renderHunger(renderer, health, screenWidth - 10 - (9 * 10), screenHeight - 30);
            }
        }
        
        // Render hotbar centered bottom
        renderHotbar(renderer, screenWidth, screenHeight);
        
        // Render root panel (debug info, etc.)
        super.render(renderer);
    }
    
    private void renderHearts(UIRenderer renderer, HealthComponent health, float x, float y) {
        float currentHealth = health.getHealth();
        float maxHealth = health.getMaxHealth();
        int totalHearts = 10; // Standard 10 hearts for 20 HP
        float iconSize = 9;
        float spacing = 8;
        
        for (int i = 0; i < totalHearts; i++) {
            float heartValue = (currentHealth - (i * 2));
            Texture2D texture;
            
            if (heartValue >= 2.0f) {
                texture = heartFullTexture;
            } else if (heartValue >= 1.0f) {
                texture = heartHalfTexture;
            } else {
                texture = heartEmptyTexture;
            }
            
            if (texture != null) {
                renderer.drawTexturedRect(x + (i * spacing), y, iconSize, iconSize, texture);
            }
        }
    }
    
    private void renderHunger(UIRenderer renderer, HealthComponent health, float x, float y) {
        float currentHunger = health.getHunger();
        float maxHunger = health.getMaxHunger();
        int totalHunger = 10; // Standard 10 hunger icons for 20 hunger
        float iconSize = 9;
        float spacing = 8;
        
        for (int i = 0; i < totalHunger; i++) {
            float hungerValue = (currentHunger - (i * 2));
            Texture2D texture;
            
            if (hungerValue >= 2.0f) {
                texture = hungerFullTexture;
            } else if (hungerValue >= 1.0f) {
                texture = hungerHalfTexture;
            } else {
                texture = hungerEmptyTexture;
            }
            
            if (texture != null) {
                renderer.drawTexturedRect(x + (i * spacing), y, iconSize, iconSize, texture);
            }
        }
    }
    
    private void renderHotbar(UIRenderer renderer, int screenWidth, int screenHeight) {
        // Hotbar dimensions (182x22 in Minecraft, scaled appropriately)
        float hotbarWidth = 182;
        float hotbarHeight = 22;
        float hotbarX = (screenWidth / 2f) - (hotbarWidth / 2f);
        float hotbarY = screenHeight - hotbarHeight - 10;
        
        // Draw hotbar background
        if (hotbarTexture != null) {
            renderer.drawTexturedRect(hotbarX, hotbarY, hotbarWidth, hotbarHeight, hotbarTexture);
        }
        
        // Draw hotbar slots and items
        float slotSize = 16;
        float slotSpacing = 20;
        float slotOffsetX = 3;
        float slotOffsetY = 3;
        
        int selectedSlot = 0;
        if (playerEntity != null) {
            InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
            if (inventory != null) {
                selectedSlot = inventory.getInventory().getSelectedSlot();
            }
        }
        
        for (int i = 0; i < Constants.Inventory.HOTBAR_SIZE; i++) {
            float slotX = hotbarX + slotOffsetX + (i * slotSpacing);
            float slotY = hotbarY + slotOffsetY;
            
            // Draw selection highlight
            if (i == selectedSlot && hotbarSelectionTexture != null) {
                renderer.drawTexturedRect(slotX - 1, slotY - 1, slotSize + 2, slotSize + 2, hotbarSelectionTexture);
            }
            
            // Render item in slot
            if (hotbarSlots[i] != null) {
                // Position the ItemSlot widget for rendering
                hotbarSlots[i].setPosition(slotX, slotY);
                hotbarSlots[i].setSize(slotSize, slotSize);
                hotbarSlots[i].render(renderer, 0, 0);
            }
        }
    }

    @Override
    protected void updateCursorMode() {
        // Hide cursor for in-game HUD
    }
}
