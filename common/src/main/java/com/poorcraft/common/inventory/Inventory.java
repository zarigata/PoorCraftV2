package com.poorcraft.common.inventory;

import java.util.Arrays;

import com.poorcraft.common.Constants;

public class Inventory {

    private final ItemStack[] slots;
    private final ItemStack[] hotbar;
    private int selectedHotbarSlot;

    public Inventory() {
        this.slots = new ItemStack[Constants.Inventory.INVENTORY_SIZE];
        this.hotbar = new ItemStack[Constants.Inventory.HOTBAR_SIZE];
        Arrays.fill(slots, ItemStack.EMPTY.copy());
        Arrays.fill(hotbar, ItemStack.EMPTY.copy());
        this.selectedHotbarSlot = 0;
    }

    public ItemStack getSlot(int index) {
        validateSlotIndex(index);
        return slots[index];
    }

    public void setSlot(int index, ItemStack stack) {
        validateSlotIndex(index);
        slots[index] = stack == null ? ItemStack.EMPTY.copy() : stack;
        if (index < hotbar.length) {
            hotbar[index] = slots[index];
        }
    }

    public ItemStack getHotbarSlot(int index) {
        validateHotbarIndex(index);
        return hotbar[index];
    }

    public void setHotbarSlot(int index, ItemStack stack) {
        validateHotbarIndex(index);
        hotbar[index] = stack == null ? ItemStack.EMPTY.copy() : stack;
        slots[index] = hotbar[index];
    }

    public int getSelectedSlot() {
        return selectedHotbarSlot;
    }

    public void setSelectedSlot(int slot) {
        validateHotbarIndex(slot);
        this.selectedHotbarSlot = slot;
    }

    public ItemStack getSelectedItem() {
        return hotbar[selectedHotbarSlot];
    }

    public ItemStack addItem(ItemStack stack) {
        if (stack == null || stack.isEmpty()) {
            return ItemStack.EMPTY.copy();
        }

        int remaining = stack.getCount();
        ItemType itemType = stack.getItemType();

        for (int i = 0; i < slots.length; i++) {
            ItemStack existing = slots[i];
            if (existing.isEmpty()) {
                continue;
            }
            if (existing.getItemType() == itemType && existing.canStack(stack)) {
                int overflow = existing.addToStack(remaining);
                remaining = overflow;
                if (remaining <= 0) {
                    return ItemStack.EMPTY.copy();
                }
            }
        }

        for (int i = 0; i < slots.length && remaining > 0; i++) {
            ItemStack existing = slots[i];
            if (!existing.isEmpty()) {
                continue;
            }
            int toAdd = Math.min(itemType.getMaxStackSize(), remaining);
            ItemStack newStack = new ItemStack(itemType, toAdd);
            setSlot(i, newStack);
            remaining -= toAdd;
        }

        if (remaining <= 0) {
            return ItemStack.EMPTY.copy();
        }
        return new ItemStack(itemType, remaining);
    }

    public int removeItem(ItemType type, int count) {
        if (type == null || type == ItemType.AIR || count <= 0) {
            return 0;
        }
        int remaining = count;
        for (int i = 0; i < slots.length && remaining > 0; i++) {
            ItemStack stack = slots[i];
            if (stack.getItemType() != type || stack.isEmpty()) {
                continue;
            }
            int removed = stack.removeFromStack(remaining);
            remaining -= removed;
            if (stack.isEmpty()) {
                setSlot(i, ItemStack.EMPTY.copy());
            }
        }
        return count - remaining;
    }

    public boolean hasItem(ItemType type, int count) {
        if (type == null || type == ItemType.AIR || count <= 0) {
            return false;
        }
        int total = 0;
        for (ItemStack stack : slots) {
            if (stack.getItemType() == type) {
                total += stack.getCount();
                if (total >= count) {
                    return true;
                }
            }
        }
        return false;
    }

    public void clear() {
        Arrays.fill(slots, ItemStack.EMPTY.copy());
        Arrays.fill(hotbar, ItemStack.EMPTY.copy());
        selectedHotbarSlot = 0;
    }

    public int getSlotCount() {
        return slots.length;
    }

    private void validateSlotIndex(int index) {
        if (index < 0 || index >= slots.length) {
            throw new IndexOutOfBoundsException("Slot index out of range: " + index);
        }
    }

    private void validateHotbarIndex(int index) {
        if (index < 0 || index >= hotbar.length) {
            throw new IndexOutOfBoundsException("Hotbar index out of range: " + index);
        }
    }
}
