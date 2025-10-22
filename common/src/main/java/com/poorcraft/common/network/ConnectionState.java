package com.poorcraft.common.network;

/**
 * Represents the current state of a network connection.
 * <p>
 * The connection state determines which packets are allowed and how they are handled.
 * Transitions between states are managed by the server during the handshake and login process.
 */
public enum ConnectionState {
    /**
     * Initial state for new connections.
     * <p>
     * Only handshake packets are allowed in this state.
     */
    HANDSHAKE(0),

    /**
     * State during the login process.
     * <p>
     * Login packets are exchanged to authenticate the client and establish the connection.
     */
    LOGIN(1),

    /**
     * State during active gameplay.
     * <p>
     * All game-related packets are handled in this state.
     */
    PLAY(2),

    /**
     * State for disconnected or closing connections.
     * <p>
     * No packets should be processed in this state.
     */
    DISCONNECTED(3);

    private final int id;

    ConnectionState(int id) {
        this.id = id;
    }

    /**
     * Gets the numeric ID for this connection state.
     *
     * @return the state ID
     */
    public int getId() {
        return id;
    }

    /**
     * Gets the ConnectionState corresponding to the given ID.
     *
     * @param id the state ID
     * @return the corresponding ConnectionState
     * @throws IllegalArgumentException if no state exists for the given ID
     */
    public static ConnectionState getById(int id) {
        for (ConnectionState state : values()) {
            if (state.id == id) {
                return state;
            }
        }
        throw new IllegalArgumentException("Unknown connection state ID: " + id);
    }
}
