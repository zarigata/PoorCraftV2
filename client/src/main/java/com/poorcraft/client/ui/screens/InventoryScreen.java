package com.poorcraft.client.ui.screens;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.ItemSlot;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.common.Constants;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.InventoryComponent;
import com.poorcraft.common.inventory.ItemStack;

import java.io.IOException;

import static org.lwjgl.glfw.GLFW.*;

/**
 * Inventory screen for managing player inventory with textured background and interactive slots.
 */
public class InventoryScreen extends UIScreen {
    private final TextureManager textureManager;
    private final ClientNetworkManager networkManager;
    private Entity playerEntity;
    
    // UI Textures
    private Texture2D inventoryBackgroundTexture;
    
    // UI Elements
    private Panel rootPanel;
    private ItemSlot[] mainInventorySlots;
    private ItemSlot[] hotbarSlots;
    private ItemStack heldStack;
    private int heldSlotIndex;
    
    public InventoryScreen(UIManager uiManager, Configuration config, TextureManager textureManager, ClientNetworkManager networkManager) {
        super(uiManager, config);
        this.textureManager = textureManager;
        this.networkManager = networkManager;
        this.heldStack = ItemStack.EMPTY;
        this.heldSlotIndex = -1;
    }
    
    public void setPlayerEntity(Entity playerEntity) {
        this.playerEntity = playerEntity;
    }

    @Override
    public void onShow() {
        super.onShow();
        loadUITextures();
        heldStack = ItemStack.EMPTY;
        heldSlotIndex = -1;
    }
    
    private void loadUITextures() {
        try {
            inventoryBackgroundTexture = textureManager.loadTexture("ui/inventory_background.png");
        } catch (IOException e) {
            System.err.println("Failed to load inventory UI textures: " + e.getMessage());
        }
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        // Root panel is full screen transparent
        rootPanel = new Panel(0, 0, screenWidth, screenHeight);
        rootPanel.setBackgroundColor(Color.TRANSPARENT);

        // Create inventory slots (27 main + 9 hotbar = 36 total)
        mainInventorySlots = new ItemSlot[27]; // 3 rows of 9
        hotbarSlots = new ItemSlot[Constants.Inventory.HOTBAR_SIZE];
        
        // Initialize all slots
        for (int i = 0; i < 27; i++) {
            mainInventorySlots[i] = new ItemSlot(0, 0, Constants.UI.INVENTORY_SLOT_SIZE);
            mainInventorySlots[i].setItemStack(ItemStack.EMPTY);
            setupSlotHandlers(mainInventorySlots[i], i + 9); // Main inventory starts at index 9
        }
        
        for (int i = 0; i < Constants.Inventory.HOTBAR_SIZE; i++) {
            hotbarSlots[i] = new ItemSlot(0, 0, Constants.UI.INVENTORY_SLOT_SIZE);
            hotbarSlots[i].setItemStack(ItemStack.EMPTY);
            setupSlotHandlers(hotbarSlots[i], i); // Hotbar is indices 0-8
        }

        setRootWidget(rootPanel);
    }
    
    private void setupSlotHandlers(ItemSlot slot, int slotIndex) {
        // Left click: swap/move
        slot.setOnClick(() -> handleLeftClick(slotIndex));
        
        // Right click: split/place one
        slot.setOnRightClick(() -> handleRightClick(slotIndex));
        
        // Shift-click: quick move
        // Note: This would need shift detection in the input system
    }
    
    private void handleLeftClick(int slotIndex) {
        if (playerEntity == null) return;
        
        InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
        if (inventory == null) return;
        
        ItemStack slotStack = inventory.getInventory().getSlot(slotIndex);
        
        if (heldStack.isEmpty() && !slotStack.isEmpty()) {
            // Pick up stack from slot
            heldStack = slotStack.copy();
            inventory.getInventory().setSlot(slotIndex, ItemStack.EMPTY);
            heldSlotIndex = slotIndex;
        } else if (!heldStack.isEmpty() && slotStack.isEmpty()) {
            // Place held stack in empty slot
            inventory.getInventory().setSlot(slotIndex, heldStack.copy());
            heldStack = ItemStack.EMPTY;
            heldSlotIndex = -1;
        } else if (!heldStack.isEmpty() && !slotStack.isEmpty()) {
            // Swap stacks
            if (heldStack.getItemType() == slotStack.getItemType() && slotStack.canStack(heldStack)) {
                // Stack items
                int maxStack = slotStack.getItemType().getMaxStackSize();
                int totalCount = heldStack.getCount() + slotStack.getCount();
                
                if (totalCount <= maxStack) {
                    // All items fit in slot
                    slotStack.setCount(totalCount);
                    inventory.getInventory().setSlot(slotIndex, slotStack);
                    heldStack = ItemStack.EMPTY;
                    heldSlotIndex = -1;
                } else {
                    // Partial stack
                    slotStack.setCount(maxStack);
                    heldStack.setCount(totalCount - maxStack);
                    inventory.getInventory().setSlot(slotIndex, slotStack);
                }
            } else {
                // Swap different items
                ItemStack temp = slotStack.copy();
                inventory.getInventory().setSlot(slotIndex, heldStack.copy());
                heldStack = temp;
            }
        }
        
        // Send inventory update to server if connected
        if (networkManager != null && networkManager.isConnected()) {
            // TODO: Send inventory update packet
        }
    }
    
    private void handleRightClick(int slotIndex) {
        if (playerEntity == null) return;
        
        InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
        if (inventory == null) return;
        
        ItemStack slotStack = inventory.getInventory().getSlot(slotIndex);
        
        if (heldStack.isEmpty() && !slotStack.isEmpty()) {
            // Pick up half the stack (rounded up)
            int halfCount = (int) Math.ceil(slotStack.getCount() / 2.0);
            heldStack = new ItemStack(slotStack.getItemType(), halfCount);
            slotStack.setCount(slotStack.getCount() - halfCount);
            
            if (slotStack.getCount() <= 0) {
                inventory.getInventory().setSlot(slotIndex, ItemStack.EMPTY);
            } else {
                inventory.getInventory().setSlot(slotIndex, slotStack);
            }
            heldSlotIndex = slotIndex;
        } else if (!heldStack.isEmpty() && slotStack.isEmpty()) {
            // Place one item
            inventory.getInventory().setSlot(slotIndex, new ItemStack(heldStack.getItemType(), 1));
            heldStack.setCount(heldStack.getCount() - 1);
            
            if (heldStack.getCount() <= 0) {
                heldStack = ItemStack.EMPTY;
                heldSlotIndex = -1;
            }
        } else if (!heldStack.isEmpty() && !slotStack.isEmpty()) {
            // Add one to stack if same type
            if (heldStack.getItemType() == slotStack.getItemType() && slotStack.canStack(heldStack)) {
                int maxStack = slotStack.getItemType().getMaxStackSize();
                if (slotStack.getCount() < maxStack) {
                    slotStack.setCount(slotStack.getCount() + 1);
                    inventory.getInventory().setSlot(slotIndex, slotStack);
                    heldStack.setCount(heldStack.getCount() - 1);
                    
                    if (heldStack.getCount() <= 0) {
                        heldStack = ItemStack.EMPTY;
                        heldSlotIndex = -1;
                    }
                }
            }
        }
        
        // Send inventory update to server if connected
        if (networkManager != null && networkManager.isConnected()) {
            // TODO: Send inventory update packet
        }
    }
    
    @Override
    public void update(float dt) {
        super.update(dt);
        
        // Update slots from player inventory
        if (playerEntity != null) {
            InventoryComponent inventory = playerEntity.getComponent(InventoryComponent.class);
            if (inventory != null) {
                // Update main inventory slots (indices 9-35)
                for (int i = 0; i < 27; i++) {
                    ItemStack stack = inventory.getInventory().getSlot(i + 9);
                    mainInventorySlots[i].setItemStack(stack);
                }
                
                // Update hotbar slots (indices 0-8)
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
        
        // Inventory background dimensions (176x166 standard Minecraft size)
        float bgWidth = 176;
        float bgHeight = 166;
        float bgX = (screenWidth / 2f) - (bgWidth / 2f);
        float bgY = (screenHeight / 2f) - (bgHeight / 2f);
        
        // Draw inventory background
        if (inventoryBackgroundTexture != null) {
            renderer.drawTexturedRect(bgX, bgY, bgWidth, bgHeight, inventoryBackgroundTexture);
        }
        
        // Render inventory slots
        renderInventorySlots(renderer, bgX, bgY);
        
        // Render held item (cursor)
        if (!heldStack.isEmpty()) {
            double[] mouseX = new double[1];
            double[] mouseY = new double[1];
            // Get mouse position from input manager if available
            // For now, render at a fixed position or skip
            // TODO: Get actual mouse position
        }
        
        // Render root panel
        super.render(renderer);
    }
    
    private void renderInventorySlots(UIRenderer renderer, float bgX, float bgY) {
        float slotSize = Constants.UI.INVENTORY_SLOT_SIZE;
        float slotSpacing = 18;
        
        // Main inventory grid (3 rows of 9)
        float mainStartX = bgX + 8;
        float mainStartY = bgY + 84;
        
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 9; col++) {
                int index = row * 9 + col;
                float slotX = mainStartX + (col * slotSpacing);
                float slotY = mainStartY + (row * slotSpacing);
                
                if (mainInventorySlots[index] != null) {
                    mainInventorySlots[index].setPosition(slotX, slotY);
                    mainInventorySlots[index].setSize(slotSize, slotSize);
                    mainInventorySlots[index].render(renderer, 0, 0);
                }
            }
        }
        
        // Hotbar (1 row of 9)
        float hotbarStartX = bgX + 8;
        float hotbarStartY = bgY + 142;
        
        for (int i = 0; i < Constants.Inventory.HOTBAR_SIZE; i++) {
            float slotX = hotbarStartX + (i * slotSpacing);
            float slotY = hotbarStartY;
            
            if (hotbarSlots[i] != null) {
                hotbarSlots[i].setPosition(slotX, slotY);
                hotbarSlots[i].setSize(slotSize, slotSize);
                hotbarSlots[i].render(renderer, 0, 0);
            }
        }
    }
    
    @Override
    public boolean handleInput(InputManager input) {
        // Close on E or ESC
        if (input.keyPressed(GLFW_KEY_E) || input.keyPressed(GLFW_KEY_ESCAPE)) {
            uiManager.setState(UIState.IN_GAME);
            return true;
        }
        
        return super.handleInput(input);
    }
    
    @Override
    protected boolean shouldConsumeInput() {
        return true; // Always consume input when inventory is open
    }

    @Override
    protected void updateCursorMode() {
        // Show cursor for inventory
    }
}
