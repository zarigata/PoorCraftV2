package com.poorcraft.common.network;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Supplier;

/**
 * Registry for packet types and their factories.
 * <p>
 * Maintains bidirectional mappings between packet IDs and packet classes,
 * with separate registries for clientbound and serverbound packets.
 * Ensures no duplicate IDs and provides packet instantiation.
 */
public class PacketRegistry {
    private final Map<Integer, Supplier<? extends Packet>> clientboundPackets = new HashMap<>();
    private final Map<Integer, Supplier<? extends Packet>> serverboundPackets = new HashMap<>();
    private final Map<Class<? extends Packet>, Integer> packetToId = new HashMap<>();

    /**
     * Registers a clientbound packet type.
     * <p>
     * Clientbound packets are sent from server to client.
     *
     * @param id the packet ID
     * @param packetClass the packet class
     * @param factory supplier for creating packet instances
     * @throws IllegalArgumentException if the ID is already registered
     */
    public void registerClientbound(int id, Class<? extends Packet> packetClass, Supplier<? extends Packet> factory) {
        if (clientboundPackets.containsKey(id)) {
            throw new IllegalArgumentException("Clientbound packet ID " + id + " is already registered");
        }

        clientboundPackets.put(id, factory);
        packetToId.put(packetClass, id);
    }

    /**
     * Registers a serverbound packet type.
     * <p>
     * Serverbound packets are sent from client to server.
     *
     * @param id the packet ID
     * @param packetClass the packet class
     * @param factory supplier for creating packet instances
     * @throws IllegalArgumentException if the ID is already registered
     */
    public void registerServerbound(int id, Class<? extends Packet> packetClass, Supplier<? extends Packet> factory) {
        if (serverboundPackets.containsKey(id)) {
            throw new IllegalArgumentException("Serverbound packet ID " + id + " is already registered");
        }

        serverboundPackets.put(id, factory);
        packetToId.put(packetClass, id);
    }

    /**
     * Creates a packet instance from the given ID and direction.
     *
     * @param id the packet ID
     * @param clientbound true if this is a clientbound packet, false for serverbound
     * @return a new packet instance, or null if not found
     */
    public Packet createPacket(int id, boolean clientbound) {
        Supplier<? extends Packet> factory = clientbound ?
            clientboundPackets.get(id) : serverboundPackets.get(id);

        return factory != null ? factory.get() : null;
    }

    /**
     * Gets the packet ID for the given packet class.
     *
     * @param packetClass the packet class
     * @return the packet ID, or -1 if not found
     */
    public int getPacketId(Class<? extends Packet> packetClass) {
        return packetToId.getOrDefault(packetClass, -1);
    }

    /**
     * Checks if a packet ID is registered for the given direction.
     *
     * @param id the packet ID
     * @param clientbound true if checking clientbound, false for serverbound
     * @return true if registered, false otherwise
     */
    public boolean isRegistered(int id, boolean clientbound) {
        return clientbound ? clientboundPackets.containsKey(id) : serverboundPackets.containsKey(id);
    }

    /**
     * Gets the number of registered clientbound packets.
     *
     * @return the count
     */
    public int getClientboundCount() {
        return clientboundPackets.size();
    }

    /**
     * Gets the number of registered serverbound packets.
     *
     * @return the count
     */
    public int getServerboundCount() {
        return serverboundPackets.size();
    }

    /**
     * Registers all standard packet types with their IDs.
     * <p>
     * This method registers all packets defined in the protocol specification.
     */
    public void registerAllPackets() {
        // Handshake packets (0x00-0x02)
        registerClientbound(0x00, com.poorcraft.common.network.packet.HandshakePacket.class,
            com.poorcraft.common.network.packet.HandshakePacket::new);
        registerServerbound(0x00, com.poorcraft.common.network.packet.HandshakePacket.class,
            com.poorcraft.common.network.packet.HandshakePacket::new);

        // Login packets (0x01-0x02)
        registerServerbound(0x01, com.poorcraft.common.network.packet.LoginStartPacket.class,
            com.poorcraft.common.network.packet.LoginStartPacket::new);
        registerClientbound(0x02, com.poorcraft.common.network.packet.LoginSuccessPacket.class,
            com.poorcraft.common.network.packet.LoginSuccessPacket::new);

        // Disconnect packet (0x03)
        registerClientbound(0x03, com.poorcraft.common.network.packet.DisconnectPacket.class,
            com.poorcraft.common.network.packet.DisconnectPacket::new);

        // Keep-alive packet (0x04)
        registerClientbound(0x04, com.poorcraft.common.network.packet.KeepAlivePacket.class,
            com.poorcraft.common.network.packet.KeepAlivePacket::new);
        registerServerbound(0x04, com.poorcraft.common.network.packet.KeepAlivePacket.class,
            com.poorcraft.common.network.packet.KeepAlivePacket::new);

        // World/chunk packets (0x05-0x07)
        registerClientbound(0x05, com.poorcraft.common.network.packet.ChunkDataPacket.class,
            com.poorcraft.common.network.packet.ChunkDataPacket::new);
        registerClientbound(0x06, com.poorcraft.common.network.packet.BlockUpdatePacket.class,
            com.poorcraft.common.network.packet.BlockUpdatePacket::new);
        registerClientbound(0x07, com.poorcraft.common.network.packet.MultiBlockUpdatePacket.class,
            com.poorcraft.common.network.packet.MultiBlockUpdatePacket::new);

        // Entity packets (0x08-0x0B)
        registerClientbound(0x08, com.poorcraft.common.network.packet.EntitySpawnPacket.class,
            com.poorcraft.common.network.packet.EntitySpawnPacket::new);
        registerClientbound(0x09, com.poorcraft.common.network.packet.EntityPositionPacket.class,
            com.poorcraft.common.network.packet.EntityPositionPacket::new);
        registerClientbound(0x0A, com.poorcraft.common.network.packet.EntityVelocityPacket.class,
            com.poorcraft.common.network.packet.EntityVelocityPacket::new);
        registerClientbound(0x0B, com.poorcraft.common.network.packet.EntityRemovePacket.class,
            com.poorcraft.common.network.packet.EntityRemovePacket::new);

        // Player movement packets (0x0C-0x0E)
        registerServerbound(0x0C, com.poorcraft.common.network.packet.PlayerPositionPacket.class,
            com.poorcraft.common.network.packet.PlayerPositionPacket::new);
        registerServerbound(0x0D, com.poorcraft.common.network.packet.PlayerLookPacket.class,
            com.poorcraft.common.network.packet.PlayerLookPacket::new);
        registerServerbound(0x0E, com.poorcraft.common.network.packet.PlayerPositionLookPacket.class,
            com.poorcraft.common.network.packet.PlayerPositionLookPacket::new);

        // Player interaction packets (0x0F-0x11)
        registerServerbound(0x0F, com.poorcraft.common.network.packet.PlayerDiggingPacket.class,
            com.poorcraft.common.network.packet.PlayerDiggingPacket::new);
        registerServerbound(0x10, com.poorcraft.common.network.packet.PlayerBlockPlacementPacket.class,
            com.poorcraft.common.network.packet.PlayerBlockPlacementPacket::new);
        registerClientbound(0x11, com.poorcraft.common.network.packet.InventoryUpdatePacket.class,
            com.poorcraft.common.network.packet.InventoryUpdatePacket::new);

        // Chat packet (0x12)
        registerClientbound(0x12, ChatMessagePacket.class, ChatMessagePacket::new);
        registerClientbound(0x13, ModListPacket.class, ModListPacket::new);
        registerServerbound(0x14, ModRequestPacket.class, ModRequestPacket::new);
        registerClientbound(0x15, ModDataPacket.class, ModDataPacket::new);
        registerServerbound(0x12, com.poorcraft.common.network.packet.ChatMessagePacket.class,
            com.poorcraft.common.network.packet.ChatMessagePacket::new);
    }
}
