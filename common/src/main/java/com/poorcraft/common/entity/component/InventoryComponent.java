package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;
import com.poorcraft.common.inventory.Inventory;
import com.poorcraft.common.inventory.ItemStack;

public class InventoryComponent implements Component {

    private final Inventory inventory;

    public InventoryComponent() {
        this.inventory = new Inventory();
    }

    public Inventory getInventory() {
        return inventory;
    }

    public ItemStack getSelectedItem() {
        return inventory.getSelectedItem();
    }

    public void selectHotbarSlot(int slot) {
        inventory.setSelectedSlot(slot);
    }

    public ItemStack addItem(ItemStack stack) {
        return inventory.addItem(stack);
    }
}
