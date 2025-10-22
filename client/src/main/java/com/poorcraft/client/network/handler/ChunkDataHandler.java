package com.poorcraft.client.network.handler;

import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.world.World;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ChunkDataPacket;
import org.slf4j.Logger;

/**
 * Handles chunk data packets from server.
 * <p>
 * Updates client world with received chunk data and triggers meshing.
 */
public class ChunkDataHandler implements PacketHandler<ChunkDataPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ChunkDataHandler.class);

    private final World world;

    public ChunkDataHandler(World world) {
        this.world = world;
    }

    @Override
    public void handle(ChunkDataPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("ChunkDataHandler received non-client connection");
            return;
        }

        LOGGER.debug("Received chunk data for ({}, {}), full={}",
            packet.getChunkX(), packet.getChunkZ(), packet.isFullChunk());
        try {
            world.addNetworkChunk(packet.getChunkX(), packet.getChunkZ(), packet.getChunkData());
            LOGGER.debug("Processed chunk data for ({}, {})", packet.getChunkX(), packet.getChunkZ());
        } catch (Exception e) {
            LOGGER.error("Failed to process chunk data for ({}, {})", packet.getChunkX(), packet.getChunkZ(), e);
        }
    }
}
