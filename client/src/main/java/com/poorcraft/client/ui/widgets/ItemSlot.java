package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.client.resource.TextureManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;
import com.poorcraft.common.inventory.ItemStack;
import com.poorcraft.common.inventory.ItemType;

import java.util.HashMap;
import java.util.Map;

/**
 * Item slot widget for displaying inventory items with real textures.
 */
public class ItemSlot extends Widget implements Panel.FocusableWidget {
    private static TextureManager textureManager;
    private static final Map<ItemType, String> ITEM_TEXTURE_MAP = new HashMap<>();
    
    static {
        // Map item types to their texture paths (using block textures for now)
        ITEM_TEXTURE_MAP.put(ItemType.STONE, "blocks/stone.png");
        ITEM_TEXTURE_MAP.put(ItemType.DIRT, "blocks/dirt.png");
        ITEM_TEXTURE_MAP.put(ItemType.GRASS, "blocks/grass_top.png");
        ITEM_TEXTURE_MAP.put(ItemType.SAND, "blocks/sand.png");
        ITEM_TEXTURE_MAP.put(ItemType.WOOD, "blocks/wood_top.png");
        ITEM_TEXTURE_MAP.put(ItemType.LEAVES, "blocks/leaves.png");
    }
    
    /**
     * Set the global texture manager for all ItemSlot instances.
     * Should be called once during initialization.
     */
    public static void setTextureManager(TextureManager manager) {
        textureManager = manager;
    }
    private ItemStack itemStack;
    private boolean selected;
    private boolean hovered;
    private Runnable onClick;
    private Runnable onDoubleClick;
    private Runnable onRightClick;
    private int backgroundColor;
    private int hoverColor;
    private int selectedColor;
    private boolean focused;

    public ItemSlot(float x, float y, float size) {
        super(x, y, size, size);
        this.itemStack = ItemStack.EMPTY;
        this.selected = false;
        this.hovered = false;
        this.focused = false;
        this.backgroundColor = Color.rgba(100, 100, 100, 128);
        this.hoverColor = Color.rgba(150, 150, 150, 180);
        this.selectedColor = Color.rgba(200, 200, 0, 180);
    }

    public void setItemStack(ItemStack itemStack) {
        this.itemStack = itemStack != null ? itemStack : ItemStack.EMPTY;
    }

    public ItemStack getItemStack() {
        return itemStack;
    }

    public void setSelected(boolean selected) {
        this.selected = selected;
    }

    public boolean isSelected() {
        return selected;
    }

    public void setOnClick(Runnable onClick) {
        this.onClick = onClick;
    }

    public void setOnDoubleClick(Runnable onDoubleClick) {
        this.onDoubleClick = onDoubleClick;
    }

    public void setOnRightClick(Runnable onRightClick) {
        this.onRightClick = onRightClick;
    }

    @Override
    public boolean hasFocus() {
        return focused;
    }

    @Override
    public void setFocus(boolean focused) {
        this.focused = focused;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible()) {
            return;
        }

        float x = getX();
        float y = getY();
        float size = getWidth();

        // Choose background color (subtle)
        int bgColor;
        if (selected) {
            bgColor = selectedColor;
        } else if (hovered || focused) {
            bgColor = hoverColor;
        } else {
            bgColor = Color.rgba(139, 139, 139, 80); // Subtle gray background
        }

        // Draw slot background
        renderer.drawRect(x, y, size, size, bgColor);

        // Draw item icon
        if (itemStack != null && !itemStack.isEmpty()) {
            Texture2D itemTexture = getItemTexture(itemStack.getItemType());
            
            if (itemTexture != null) {
                // Draw item texture
                float itemSize = size * 0.9f;
                float itemX = x + (size - itemSize) / 2.0f;
                float itemY = y + (size - itemSize) / 2.0f;
                renderer.drawTexturedRect(itemX, itemY, itemSize, itemSize, itemTexture);
            } else {
                // Fallback: draw colored rectangle if texture not available
                int itemColor = getItemColor(itemStack);
                float itemSize = size * 0.8f;
                float itemX = x + (size - itemSize) / 2.0f;
                float itemY = y + (size - itemSize) / 2.0f;
                renderer.drawRect(itemX, itemY, itemSize, itemSize, itemColor);
            }

            // Draw item count if > 1 with shadow for readability
            if (itemStack.getCount() > 1) {
                String countText = String.valueOf(itemStack.getCount());
                float textWidth = renderer.measureText(countText);
                float textX = x + size - textWidth - 2.0f;
                float textY = y + size - 2.0f;
                
                // Draw shadow
                renderer.drawText(countText, textX + 1, textY + 1, Color.rgba(0, 0, 0, 180));
                // Draw text
                renderer.drawText(countText, textX, textY, Color.WHITE);
            }
        }

        // Draw slot border (subtle)
        renderer.drawRect(x, y, size, 1, Color.rgba(55, 55, 55, 255));
        renderer.drawRect(x, y + size - 1, size, 1, Color.rgba(255, 255, 255, 100));
        renderer.drawRect(x, y, 1, size, Color.rgba(55, 55, 55, 255));
        renderer.drawRect(x + size - 1, y, 1, size, Color.rgba(255, 255, 255, 100));
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        hovered = contains(mouseX, mouseY);

        // Handle left click
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (hovered) {
                if (onClick != null) {
                    onClick.run();
                }
                focused = true;
                return true;
            } else if (focused) {
                focused = false;
                return true;
            }
        }

        // Handle right click
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_RIGHT)) {
            if (hovered && onRightClick != null) {
                onRightClick.run();
                return true;
            }
        }

        // Handle double click
        if (input.mouseDoubleClicked(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (hovered && onDoubleClick != null) {
                onDoubleClick.run();
                return true;
            }
        }

        // Update focus state
        if (hovered && !focused) {
            focused = true;
        } else if (!hovered) {
            focused = false;
        }

        return false;
    }

    /**
     * Get the texture for an item type.
     */
    private Texture2D getItemTexture(ItemType itemType) {
        if (textureManager == null || itemType == null || itemType == ItemType.AIR) {
            return null;
        }
        
        String texturePath = ITEM_TEXTURE_MAP.get(itemType);
        if (texturePath == null) {
            return null;
        }
        
        try {
            return textureManager.getTexture(texturePath);
        } catch (Exception e) {
            // Texture not found, will use fallback color
            return null;
        }
    }
    
    private int getItemColor(ItemStack itemStack) {
        // Fallback color based on item type name hash
        String name = itemStack.getItemType().getName();
        int hash = name.hashCode();
        int r = (hash & 0xFF0000) >> 16;
        int g = (hash & 0x00FF00) >> 8;
        int b = hash & 0x0000FF;
        return Color.rgba(r, g, b, 255);
    }
}
