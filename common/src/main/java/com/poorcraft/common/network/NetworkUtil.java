package com.poorcraft.common.network;

import io.netty.buffer.ByteBuf;
import org.slf4j.Logger;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

/**
 * Utility class for network serialization operations.
 * <p>
 * Provides consistent methods for reading and writing common data types
 * such as varints, strings, UUIDs, vectors, and byte arrays.
 * All packets should use these methods for serialization to ensure consistency.
 */
public final class NetworkUtil {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(NetworkUtil.class);

    private NetworkUtil() {
        // Prevent instantiation
    }

    /**
     * Writes a varint (variable-length integer) to the buffer.
     * <p>
     * Varints are used to efficiently encode small integers.
     * Values are encoded in 7-bit chunks with the 8th bit indicating continuation.
     *
     * @param buf the buffer to write to
     * @param value the value to write
     */
    public static void writeVarInt(ByteBuf buf, int value) {
        while ((value & 0xFFFFFF80) != 0L) {
            buf.writeByte((value & 0x7F) | 0x80);
            value >>>= 7;
        }
        buf.writeByte(value & 0x7F);
    }

    /**
     * Reads a varint from the buffer.
     *
     * @param buf the buffer to read from
     * @return the decoded value
     * @throws IllegalArgumentException if the varint is malformed or too large
     */
    public static int readVarInt(ByteBuf buf) {
        int value = 0;
        int position = 0;
        byte currentByte;

        while (true) {
            currentByte = buf.readByte();
            value |= (currentByte & 0x7F) << position;

            if ((currentByte & 0x80) != 0x80) {
                break;
            }

            position += 7;

            if (position >= 32) {
                throw new IllegalArgumentException("VarInt is too big");
            }
        }

        return value;
    }

    /**
     * Writes a string to the buffer with a varint length prefix.
     *
     * @param buf the buffer to write to
     * @param str the string to write (must not be null)
     */
    public static void writeString(ByteBuf buf, String str) {
        if (str == null) {
            throw new IllegalArgumentException("String cannot be null");
        }

        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        writeVarInt(buf, bytes.length);
        buf.writeBytes(bytes);
    }

    /**
     * Reads a string from the buffer.
     *
     * @param buf the buffer to read from
     * @return the decoded string
     */
    public static String readString(ByteBuf buf) {
        int length = readVarInt(buf);
        if (length < 0) {
            throw new IllegalArgumentException("String length cannot be negative: " + length);
        }

        if (length > com.poorcraft.common.Constants.Network.MAX_PACKET_SIZE) {
            throw new IllegalArgumentException("String length too large: " + length);
        }

        byte[] bytes = new byte[length];
        buf.readBytes(bytes);
        return new String(bytes, StandardCharsets.UTF_8);
    }

    /**
     * Writes a UUID to the buffer.
     *
     * @param buf the buffer to write to
     * @param uuid the UUID to write (must not be null)
     */
    public static void writeUUID(ByteBuf buf, UUID uuid) {
        if (uuid == null) {
            throw new IllegalArgumentException("UUID cannot be null");
        }

        buf.writeLong(uuid.getMostSignificantBits());
        buf.writeLong(uuid.getLeastSignificantBits());
    }

    /**
     * Reads a UUID from the buffer.
     *
     * @param buf the buffer to read from
     * @return the decoded UUID
     */
    public static UUID readUUID(ByteBuf buf) {
        return new UUID(buf.readLong(), buf.readLong());
    }

    /**
     * Writes a byte array to the buffer with a varint length prefix.
     *
     * @param buf the buffer to write to
     * @param bytes the byte array to write (must not be null)
     */
    public static void writeByteArray(ByteBuf buf, byte[] bytes) {
        if (bytes == null) {
            throw new IllegalArgumentException("Byte array cannot be null");
        }

        writeVarInt(buf, bytes.length);
        buf.writeBytes(bytes);
    }

    /**
     * Reads a byte array from the buffer.
     *
     * @param buf the buffer to read from
     * @return the decoded byte array
     */
    public static byte[] readByteArray(ByteBuf buf) {
        int length = readVarInt(buf);
        if (length < 0) {
            throw new IllegalArgumentException("Byte array length cannot be negative: " + length);
        }

        if (length > com.poorcraft.common.Constants.Network.MAX_PACKET_SIZE) {
            throw new IllegalArgumentException("Byte array length too large: " + length);
        }

        byte[] bytes = new byte[length];
        buf.readBytes(bytes);
        return bytes;
    }

    /**
     * Writes a 3D vector (position) to the buffer.
     * <p>
     * Each component is written as a double.
     *
     * @param buf the buffer to write to
     * @param x the x component
     * @param y the y component
     * @param z the z component
     */
    public static void writeVector3d(ByteBuf buf, double x, double y, double z) {
        buf.writeDouble(x);
        buf.writeDouble(y);
        buf.writeDouble(z);
    }

    /**
     * Reads a 3D vector from the buffer.
     *
     * @param buf the buffer to read from
     * @return an array containing {x, y, z}
     */
    public static double[] readVector3d(ByteBuf buf) {
        return new double[]{buf.readDouble(), buf.readDouble(), buf.readDouble()};
    }

    /**
     * Writes a 2D vector (rotation) to the buffer.
     * <p>
     * Each component is written as a float.
     *
     * @param buf the buffer to write to
     * @param yaw the yaw (horizontal rotation)
     * @param pitch the pitch (vertical rotation)
     */
    public static void writeVector2f(ByteBuf buf, float yaw, float pitch) {
        buf.writeFloat(yaw);
        buf.writeFloat(pitch);
    }

    /**
     * Reads a 2D vector from the buffer.
     *
     * @param buf the buffer to read from
     * @return an array containing {yaw, pitch}
     */
    public static float[] readVector2f(ByteBuf buf) {
        return new float[]{buf.readFloat(), buf.readFloat()};
    }
}
