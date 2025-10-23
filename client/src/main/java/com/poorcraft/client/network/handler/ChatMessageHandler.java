package com.poorcraft.client.network.handler;

import com.poorcraft.client.network.ClientConnection;
import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.ui.screens.ChatOverlay;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ChatMessagePacket;
import org.slf4j.Logger;

/**
 * Handles incoming chat message packets on the client.
 */
public class ChatMessageHandler implements PacketHandler<ChatMessagePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ChatMessageHandler.class);

    private final ClientNetworkManager networkManager;
    private final ChatOverlay chatOverlay;

    public ChatMessageHandler(ClientNetworkManager networkManager, ChatOverlay chatOverlay) {
        this.networkManager = networkManager;
        this.chatOverlay = chatOverlay;
    }

    @Override
    public void handle(ChatMessagePacket packet, NetworkConnection connection) {
        if (!(connection instanceof ClientConnection)) {
            LOGGER.error("ChatMessageHandler received non-client connection");
            return;
        }

        if (chatOverlay == null) {
            LOGGER.warn("Chat overlay not available to display message");
            return;
        }

        networkManager.enqueue(() -> chatOverlay.addMessage(packet.getMessage(), packet.getType()));
    }
}
