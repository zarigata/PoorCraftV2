package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Player block placement packet sent by client when placing blocks.
 * <p>
 * Contains the placement position, face, and item being placed.
 */
public class PlayerBlockPlacementPacket implements Packet {
    private long position;
    private int face;
    private int hand;
    private float cursorX;
    private float cursorY;
    private float cursorZ;

    /**
     * Default constructor for reading from buffer.
     */
    public PlayerBlockPlacementPacket() {}

    /**
     * Constructor for creating a player block placement packet.
     *
     * @param x block X coordinate
     * @param y block Y coordinate
     * @param z block Z coordinate
     * @param face the face being placed against (0-5)
     * @param hand the hand used (0=main, 1=offhand)
     * @param cursorX cursor position X
     * @param cursorY cursor position Y
     * @param cursorZ cursor position Z
     */
    public PlayerBlockPlacementPacket(int x, int y, int z, int face, int hand, float cursorX, float cursorY, float cursorZ) {
        this.position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        this.face = face;
        this.hand = hand;
        this.cursorX = cursorX;
        this.cursorY = cursorY;
        this.cursorZ = cursorZ;
    }

    @Override
    public int getPacketId() {
        return 0x10;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, (int) (position >> 38)); // x
        NetworkUtil.writeVarInt(buf, (int) (position & 0xFFF)); // y
        NetworkUtil.writeVarInt(buf, (int) ((position >> 12) & 0x3FFFFFF)); // z
        NetworkUtil.writeVarInt(buf, face);
        NetworkUtil.writeVarInt(buf, hand);
        buf.writeFloat(cursorX);
        buf.writeFloat(cursorY);
        buf.writeFloat(cursorZ);
    }

    @Override
    public void read(ByteBuf buf) {
        int x = NetworkUtil.readVarInt(buf);
        int y = NetworkUtil.readVarInt(buf);
        int z = NetworkUtil.readVarInt(buf);
        position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        face = NetworkUtil.readVarInt(buf);
        hand = NetworkUtil.readVarInt(buf);
        cursorX = buf.readFloat();
        cursorY = buf.readFloat();
        cursorZ = buf.readFloat();
    }

    /**
     * Gets the block position as a packed long.
     *
     * @return the packed position
     */
    public long getPosition() {
        return position;
    }

    /**
     * Gets the block X coordinate.
     *
     * @return the X coordinate
     */
    public int getX() {
        return (int) (position >> 38);
    }

    /**
     * Gets the block Y coordinate.
     *
     * @return the Y coordinate
     */
    public int getY() {
        return (int) (position & 0xFFF);
    }

    /**
     * Gets the block Z coordinate.
     *
     * @return the Z coordinate
     */
    public int getZ() {
        return (int) ((position >> 12) & 0x3FFFFFF);
    }

    /**
     * Gets the face being placed against.
     *
     * @return the face (0-5)
     */
    public int getFace() {
        return face;
    }

    /**
     * Gets the hand used.
     *
     * @return the hand (0=main, 1=offhand)
     */
    public int getHand() {
        return hand;
    }

    /**
     * Gets the cursor position X.
     *
     * @return the cursor X
     */
    public float getCursorX() {
        return cursorX;
    }

    /**
     * Gets the cursor position Y.
     *
     * @return the cursor Y
     */
    public float getCursorY() {
        return cursorY;
    }

    /**
     * Gets the cursor position Z.
     *
     * @return the cursor Z
     */
    public float getCursorZ() {
        return cursorZ;
    }
}
