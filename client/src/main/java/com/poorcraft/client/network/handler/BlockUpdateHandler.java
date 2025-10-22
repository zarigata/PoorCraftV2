package com.poorcraft.client.network.handler;

import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.world.World;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.BlockUpdatePacket;
import org.slf4j.Logger;

/**
 * Handles block update packets from server.
 * <p>
 * Updates client world with block changes and triggers remeshing if needed.
 */
public class BlockUpdateHandler implements PacketHandler<BlockUpdatePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(BlockUpdateHandler.class);

    private final World world;

    public BlockUpdateHandler(World world) {
        this.world = world;
    }

    @Override
    public void handle(BlockUpdatePacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("BlockUpdateHandler received non-client connection");
            return;
        }

        LOGGER.debug("Block update at ({}, {}, {}) to ID {}",
            packet.getX(), packet.getY(), packet.getZ(), packet.getBlockId());

        world.setBlock(packet.getX(), packet.getY(), packet.getZ(), packet.getBlockId());

        LOGGER.debug("Processed block update at ({}, {}, {})",
            packet.getX(), packet.getY(), packet.getZ());
    }
}
