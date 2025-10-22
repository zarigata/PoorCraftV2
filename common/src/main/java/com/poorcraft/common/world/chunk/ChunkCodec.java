package com.poorcraft.common.world.chunk;

import com.poorcraft.common.Constants;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

public final class ChunkCodec {
    private ChunkCodec() {}

    public static byte[] serialize(Chunk chunk) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        try (DataOutputStream out = new DataOutputStream(buffer)) {
            writeChunkPayload(out, chunk);
            out.flush();
            return buffer.toByteArray();
        } catch (IOException e) {
            throw new IllegalStateException("Failed to serialize chunk", e);
        }
    }

    public static Chunk deserialize(int chunkX, int chunkZ, byte[] data) {
        Chunk chunk = new Chunk(chunkX, chunkZ);
        try (DataInputStream in = new DataInputStream(new ByteArrayInputStream(data))) {
            readChunkPayload(in, chunk);
            return chunk;
        } catch (IOException e) {
            throw new IllegalStateException("Failed to deserialize chunk", e);
        }
    }

    public static byte[] encodeFullChunk(Chunk chunk) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        try (DataOutputStream out = new DataOutputStream(buffer)) {
            out.writeInt(chunk.getChunkX());
            out.writeInt(chunk.getChunkZ());
            writeChunkPayload(out, chunk);
            out.flush();
            return buffer.toByteArray();
        } catch (IOException e) {
            throw new IllegalStateException("Failed to encode chunk", e);
        }
    }

    public static Chunk decodeFullChunk(byte[] data) {
        try (DataInputStream in = new DataInputStream(new ByteArrayInputStream(data))) {
            int chunkX = in.readInt();
            int chunkZ = in.readInt();
            Chunk chunk = new Chunk(chunkX, chunkZ);
            readChunkPayload(in, chunk);
            return chunk;
        } catch (IOException e) {
            throw new IllegalStateException("Failed to decode chunk", e);
        }
    }

    private static void writeChunkPayload(DataOutputStream out, Chunk chunk) throws IOException {
        for (int y = 0; y < Constants.World.CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    out.writeShort(chunk.getBlock(x, y, z));
                }
            }
        }
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                out.writeByte(chunk.getBiome(x, z));
            }
        }
        out.writeBoolean(chunk.isGenerated());
    }

    private static void readChunkPayload(DataInputStream in, Chunk chunk) throws IOException {
        for (int y = 0; y < Constants.World.CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    int value = in.readUnsignedShort();
                    chunk.setBlock(x, y, z, value);
                }
            }
        }
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                chunk.setBiome(x, z, in.readByte());
            }
        }
        boolean generated = in.readBoolean();
        chunk.setGenerated(generated);
        chunk.updateHeightMap();
    }
}
