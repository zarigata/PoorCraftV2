package com.poorcraft.server.network.handler;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.NetworkConnection;
import com.poorcraft.common.network.PacketHandler;
import com.poorcraft.common.network.packet.ModDataPacket;
import com.poorcraft.common.network.packet.ModRequestPacket;
import com.poorcraft.server.GameServer;
import com.poorcraft.server.network.ServerConnection;
import org.slf4j.Logger;

import java.util.Arrays;
import java.util.Objects;
import java.util.regex.Pattern;

public class ModRequestHandler implements PacketHandler<ModRequestPacket> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ModRequestHandler.class);
    private static final Pattern MOD_ID_PATTERN = Pattern.compile("[a-z0-9_\\-]+");

    private final GameServer gameServer;
    private final int chunkSize;
    private final long maxModSize;

    public ModRequestHandler(GameServer gameServer) {
        this.gameServer = Objects.requireNonNull(gameServer, "gameServer");
        Configuration config = gameServer.getConfig();
        int configuredChunk = config.getInt("mods.chunkSize", 262144);
        int maxPacketSize = config.getInt("network.maxPacketSize", com.poorcraft.common.Constants.Network.MAX_PACKET_SIZE);
        this.chunkSize = Math.max(1024, Math.min(configuredChunk, Math.max(1024, maxPacketSize - 128)));
        this.maxModSize = config.getLong("mods.maxModSize", com.poorcraft.common.Constants.Network.MAX_PACKET_SIZE * 4L);
    }

    @Override
    public void handle(ModRequestPacket packet, NetworkConnection connection) {
        if (!(connection instanceof ServerConnection)) {
            LOGGER.warn("ModRequestHandler received non-server connection");
            return;
        }

        ServerConnection serverConnection = (ServerConnection) connection;
        String modId = packet.getModId();
        if (!isValidModId(modId)) {
            LOGGER.warn("Disconnecting {} for invalid mod request id '{}'", serverConnection.getRemoteAddress(), modId);
            serverConnection.sendPacket(new com.poorcraft.common.network.packet.DisconnectPacket("Invalid mod request"));
            serverConnection.disconnect("Invalid mod request");
            return;
        }

        byte[] data = gameServer.getModManager().getModData(modId);
        if (data == null) {
            LOGGER.warn("Player {} requested missing mod '{}'", serverConnection.getRemoteAddress(), modId);
            serverConnection.sendPacket(new com.poorcraft.common.network.packet.DisconnectPacket("Requested mod not available"));
            serverConnection.disconnect("Requested missing mod");
            return;
        }

        if (data.length > maxModSize) {
            LOGGER.warn("Player {} requested mod '{}' exceeding size limit {} bytes", serverConnection.getRemoteAddress(), modId, maxModSize);
            serverConnection.sendPacket(new com.poorcraft.common.network.packet.DisconnectPacket("Requested mod exceeds size limit"));
            serverConnection.disconnect("Mod too large");
            return;
        }

        String checksum = gameServer.getModManager().ensureChecksum(modId);
        int totalChunks = Math.max(1, (int) Math.ceil((double) data.length / chunkSize));
        for (int chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
            int start = chunkIndex * chunkSize;
            int length = Math.min(chunkSize, data.length - start);
            byte[] chunk = Arrays.copyOfRange(data, start, start + length);
            String chunkChecksum = chunkIndex == totalChunks - 1 ? checksum : null;
            serverConnection.sendPacket(new ModDataPacket(modId, chunkIndex, totalChunks, chunk, chunkChecksum));
        }

        LOGGER.debug("Streamed mod '{}' to {} in {} chunk(s)", modId, serverConnection.getRemoteAddress(), totalChunks);
    }

    private boolean isValidModId(String modId) {
        if (modId == null || modId.isEmpty()) {
            return false;
        }
        return MOD_ID_PATTERN.matcher(modId).matches();
    }
}
