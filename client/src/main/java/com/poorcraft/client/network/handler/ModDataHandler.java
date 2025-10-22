package com.poorcraft.client.network.handler;

import com.poorcraft.client.mod.ClientModManager;
import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ModDataPacket;
import org.slf4j.Logger;

/**
 * Handles {@link ModDataPacket} packets and forwards them to the {@link ClientModManager}.
 */
public class ModDataHandler implements PacketHandler<ModDataPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ModDataHandler.class);

    private final ClientModManager modManager;

    public ModDataHandler(ClientModManager modManager) {
        this.modManager = modManager;
    }

    @Override
    public void handle(ModDataPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("ModDataHandler received non-client connection");
            return;
        }
        modManager.handleModData(packet);
    }
}
