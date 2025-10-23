package com.poorcraft.server.network.handler;

import com.poorcraft.common.network.ConnectionState;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ChatMessagePacket;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.PlayerSession;
import com.poorcraft.server.network.ServerConnection;
import org.slf4j.Logger;

/**
 * Handles chat messages received from clients on the server.
 */
public class ChatMessageHandler implements PacketHandler<ChatMessagePacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ChatMessageHandler.class);

    private static final int MAX_MESSAGE_LENGTH = 256;

    private final GameServer gameServer;

    public ChatMessageHandler(GameServer gameServer) {
        this.gameServer = gameServer;
    }

    @Override
    public void handle(ChatMessagePacket packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection serverConnection)) {
            LOGGER.error("ChatMessageHandler received non-server connection");
            return;
        }

        if (serverConnection.getConnectionState() != ConnectionState.PLAY) {
            LOGGER.warn("Chat message received before PLAY state from {}", serverConnection.getRemoteAddress());
            return;
        }

        PlayerSession session = serverConnection.getPlayerSession();
        if (session == null) {
            LOGGER.warn("Chat message received without an associated player session");
            return;
        }

        String message = packet.getMessage();
        if (message == null || message.isBlank()) {
            return;
        }

        if (message.length() > MAX_MESSAGE_LENGTH) {
            message = message.substring(0, MAX_MESSAGE_LENGTH);
        }

        if (message.startsWith("/")) {
            processCommand(message.substring(1), session);
            return;
        }

        String formatted = "<" + session.getUsername() + "> " + message;
        ChatMessagePacket outbound = new ChatMessagePacket(formatted, 0);
        gameServer.getNetworkManager().broadcast(outbound);
        LOGGER.info("{}: {}", session.getUsername(), message);
    }

    private void processCommand(String commandLine, PlayerSession session) {
        LOGGER.info("Command from {}: /{}", session.getUsername(), commandLine);
        // TODO: Implement command execution
    }
}
