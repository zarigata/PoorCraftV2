package com.poorcraft.client.network.handler;

import com.poorcraft.client.mod.ClientModManager;
import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ModListPacket;
import org.slf4j.Logger;

/**
 * Handles {@link ModListPacket} packets by delegating to the {@link ClientModManager}.
 */
public class ModListHandler implements PacketHandler<ModListPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ModListHandler.class);

    private final ClientModManager modManager;
    private final ClientNetworkManager networkManager;

    public ModListHandler(ClientModManager modManager, ClientNetworkManager networkManager) {
        this.modManager = modManager;
        this.networkManager = networkManager;
    }

    @Override
    public void handle(ModListPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("ModListHandler received non-client connection");
            return;
        }
        networkManager.enqueue(() -> modManager.handleModList(packet));
    }
}
