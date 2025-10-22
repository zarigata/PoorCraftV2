package com.poorcraft.common.network.packet;

import com.poorcraft.common.network.NetworkUtil;
import com.poorcraft.common.network.Packet;
import io.netty.buffer.ByteBuf;

/**
 * Handshake packet for initial connection negotiation.
 * <p>
 * Sent by client to server to initiate the connection and specify
 * the intended connection state and protocol version.
 */
public class HandshakePacket implements Packet {
    private int protocolVersion;
    private String serverAddress;
    private int serverPort;
    private int nextState;

    /**
     * Default constructor for reading from buffer.
     */
    public HandshakePacket() {}

    /**
     * Constructor for creating a handshake packet.
     *
     * @param protocolVersion the protocol version
     * @param serverAddress the server address
     * @param serverPort the server port
     * @param nextState the next connection state (1 for LOGIN, 2 for PLAY)
     */
    public HandshakePacket(int protocolVersion, String serverAddress, int serverPort, int nextState) {
        this.protocolVersion = protocolVersion;
        this.serverAddress = serverAddress;
        this.serverPort = serverPort;
        this.nextState = nextState;
    }

    @Override
    public int getPacketId() {
        return 0x00;
    }

    @Override
    public void write(ByteBuf buf) {
        NetworkUtil.writeVarInt(buf, protocolVersion);
        NetworkUtil.writeString(buf, serverAddress);
        buf.writeShort(serverPort);
        NetworkUtil.writeVarInt(buf, nextState);
    }

    @Override
    public void read(ByteBuf buf) {
        protocolVersion = NetworkUtil.readVarInt(buf);
        serverAddress = NetworkUtil.readString(buf);
        serverPort = buf.readShort();
        nextState = NetworkUtil.readVarInt(buf);
    }

    /**
     * Gets the protocol version.
     *
     * @return the protocol version
     */
    public int getProtocolVersion() {
        return protocolVersion;
    }

    /**
     * Gets the server address.
     *
     * @return the server address
     */
    public String getServerAddress() {
        return serverAddress;
    }

    /**
     * Gets the server port.
     *
     * @return the server port
     */
    public int getServerPort() {
        return serverPort;
    }

    /**
     * Gets the next connection state.
     *
     * @return the next state (1 for LOGIN, 2 for PLAY)
     */
    public int getNextState() {
        return nextState;
    }
}
