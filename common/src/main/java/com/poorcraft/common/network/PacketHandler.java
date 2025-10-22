package com.poorcraft.common.network;

/**
 * Functional interface for handling network packets.
 * <p>
 * Packet handlers are registered with the network manager to process
 * incoming packets of specific types. The handler receives the packet
 * and the network connection context.
 *
 * @param <T> the packet type this handler processes
 */
@FunctionalInterface
public interface PacketHandler<T extends Packet> {
    /**
     * Handles the given packet on the specified connection.
     * <p>
     * Implementations should process the packet data and perform any necessary
     * actions such as updating game state, sending responses, or managing
     * the connection lifecycle.
     *
     * @param packet the packet to handle
     * @param connection the network connection context
     */
    void handle(T packet, NetworkConnection connection);
}
