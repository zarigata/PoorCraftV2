package com.poorcraft.common.inventory;

import java.util.Objects;

public class ItemStack {

    public static final ItemStack EMPTY = new ItemStack(ItemType.AIR, 0);

    private final ItemType itemType;
    private int count;

    public ItemStack(ItemType itemType, int count) {
        this.itemType = Objects.requireNonNull(itemType, "itemType");
        setCount(count);
    }

    public ItemType getItemType() {
        return itemType;
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        if (itemType == ItemType.AIR) {
            this.count = 0;
            return;
        }
        int max = itemType.getMaxStackSize();
        if (count < 0) {
            this.count = 0;
        } else if (max > 0) {
            this.count = Math.min(count, max);
        } else {
            this.count = count;
        }
    }

    public boolean isEmpty() {
        return itemType == ItemType.AIR || count <= 0;
    }

    public boolean isFull() {
        return !isEmpty() && count >= itemType.getMaxStackSize();
    }

    public boolean canStack(ItemStack other) {
        if (other == null || other.isEmpty()) {
            return !isFull();
        }
        if (this.isEmpty()) {
            return other.itemType.isStackable();
        }
        return itemType == other.itemType && itemType.isStackable() && !isFull();
    }

    public int addToStack(int amount) {
        if (amount <= 0 || itemType == ItemType.AIR) {
            return amount;
        }
        int max = itemType.getMaxStackSize();
        if (max <= 0) {
            count += amount;
            return 0;
        }
        int space = Math.max(0, max - count);
        int added = Math.min(space, amount);
        count += added;
        return amount - added;
    }

    public int removeFromStack(int amount) {
        if (amount <= 0 || isEmpty()) {
            return 0;
        }
        int removed = Math.min(amount, count);
        count -= removed;
        if (count <= 0) {
            count = 0;
        }
        return removed;
    }

    public ItemStack split(int amount) {
        if (amount <= 0 || isEmpty()) {
            return EMPTY.copy();
        }
        int removed = Math.min(amount, count);
        count -= removed;
        return new ItemStack(itemType, removed);
    }

    public ItemStack copy() {
        return new ItemStack(itemType, count);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof ItemStack other)) {
            return false;
        }
        return itemType == other.itemType && count == other.count;
    }

    @Override
    public int hashCode() {
        return Objects.hash(itemType, count);
    }
}
