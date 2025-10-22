package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Player digging packet sent by client when breaking blocks.
 * <p>
 * Contains the digging action, position, and face.
 */
public class PlayerDiggingPacket implements Packet {
    private int action;
    private long position;
    private int face;

    /**
     * Default constructor for reading from buffer.
     */
    public PlayerDiggingPacket() {}

    /**
     * Constructor for creating a player digging packet.
     *
     * @param action the digging action (0=start, 1=finish, 2=cancel)
     * @param x block X coordinate
     * @param y block Y coordinate
     * @param z block Z coordinate
     * @param face the face being dug (0-5)
     */
    public PlayerDiggingPacket(int action, int x, int y, int z, int face) {
        this.action = action;
        this.position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        this.face = face;
    }

    @Override
    public int getPacketId() {
        return 0x0F;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, action);
        NetworkUtil.writeVarInt(buf, (int) (position >> 38)); // x
        NetworkUtil.writeVarInt(buf, (int) (position & 0xFFF)); // y
        NetworkUtil.writeVarInt(buf, (int) ((position >> 12) & 0x3FFFFFF)); // z
        buf.writeByte(face);
    }

    @Override
    public void read(ByteBuf buf) {
        action = NetworkUtil.readVarInt(buf);
        int x = NetworkUtil.readVarInt(buf);
        int y = NetworkUtil.readVarInt(buf);
        int z = NetworkUtil.readVarInt(buf);
        position = ((long) x & 0x3FFFFFF) << 38 | ((long) z & 0x3FFFFFF) << 12 | (y & 0xFFF);
        face = buf.readByte();
    }

    /**
     * Gets the digging action.
     *
     * @return the action (0=start, 1=finish, 2=cancel)
     */
    public int getAction() {
        return action;
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
     * Gets the face being dug.
     *
     * @return the face (0-5)
     */
    public int getFace() {
        return face;
    }
}
