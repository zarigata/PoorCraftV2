package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Inventory update packet for synchronizing inventory changes.
 * <p>
 * Sent to update client inventory slots.
 */
public class InventoryUpdatePacket implements Packet {
    private int windowId;
    private int slot;
    private int itemId;
    private int count;
    private int damage;

    /**
     * Default constructor for reading from buffer.
     */
    public InventoryUpdatePacket() {}

    /**
     * Constructor for creating an inventory update packet.
     *
     * @param windowId the window ID
     * @param slot the slot index
     * @param itemId the item ID
     * @param count the item count
     * @param damage the item damage/metadata
     */
    public InventoryUpdatePacket(int windowId, int slot, int itemId, int count, int damage) {
        this.windowId = windowId;
        this.slot = slot;
        this.itemId = itemId;
        this.count = count;
        this.damage = damage;
    }

    @Override
    public int getPacketId() {
        return 0x11;
    }

    @Override
    public void write(ByteBuf buf) {
        buf.writeByte(windowId);
        buf.writeShort(slot);
        NetworkUtil.writeVarInt(buf, itemId);
        buf.writeByte(count);
        NetworkUtil.writeVarInt(buf, damage);
    }

    @Override
    public void read(ByteBuf buf) {
        windowId = buf.readByte();
        slot = buf.readShort();
        itemId = NetworkUtil.readVarInt(buf);
        count = buf.readByte();
        damage = NetworkUtil.readVarInt(buf);
    }

    /**
     * Gets the window ID.
     *
     * @return the window ID
     */
    public int getWindowId() {
        return windowId;
    }

    /**
     * Gets the slot index.
     *
     * @return the slot
     */
    public int getSlot() {
        return slot;
    }

    /**
     * Gets the item ID.
     *
     * @return the item ID
     */
    public int getItemId() {
        return itemId;
    }

    /**
     * Gets the item count.
     *
     * @return the count
     */
    public int getCount() {
        return count;
    }

    /**
     * Gets the item damage/metadata.
     *
     * @return the damage
     */
    public int getDamage() {
        return damage;
    }
}
