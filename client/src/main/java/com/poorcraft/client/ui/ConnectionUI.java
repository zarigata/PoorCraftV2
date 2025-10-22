package com.poorcraft.client.ui;

import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.common.network.packet.LoginStartPacket;
import org.slf4j.Logger;

import java.util.Scanner;
import java.util.concurrent.TimeUnit;

/**
 * Simple connection UI for establishing server connections.
 * <p>
 * Handles the connection process, login, and timeout handling.
 */
public class ConnectionUI {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ConnectionUI.class);

    private final ClientNetworkManager networkManager;

    /**
     * Creates a new connection UI.
     *
     * @param networkManager the network manager
     */
    public ConnectionUI(ClientNetworkManager networkManager) {
        this.networkManager = networkManager;
    }

    /**
     * Connects to the specified server.
     * <p>
     * Parses the server address, connects, sends login, and waits for success.
     *
     * @param serverAddress the server address in format "host:port"
     * @return true if connection and login successful, false otherwise
     */
    public boolean connect(String serverAddress) {
        LOGGER.info("Connecting to server: {}", serverAddress);

        try {
            // Parse server address
            String[] parts = serverAddress.split(":");
            if (parts.length != 2) {
                LOGGER.error("Invalid server address format: {}", serverAddress);
                return false;
            }

            String host = parts[0];
            int port = Integer.parseInt(parts[1]);

            // Connect to server
            networkManager.connect(host, port);

            // Wait a bit for connection to establish
            Thread.sleep(1000);

            if (!networkManager.isConnected()) {
                LOGGER.error("Failed to establish connection to server");
                return false;
            }

            // Get username from user input (in a real implementation, this would be a GUI)
            String username = getUsernameFromInput();

            // Send login packet
            networkManager.sendLoginStart(username);

            // Wait for login success (in a real implementation, this would be event-driven)
            int attempts = 0;
            int maxAttempts = 30; // 30 seconds timeout

            while (attempts < maxAttempts) {
                if (networkManager.getConnectionState() == com.poorcraft.common.network.ConnectionState.PLAY) {
                    LOGGER.info("Login successful");
                    return true;
                }

                Thread.sleep(1000);
                attempts++;
            }

            LOGGER.error("Login timed out");
            return false;

        } catch (Exception e) {
            LOGGER.error("Error connecting to server", e);
            return false;
        }
    }

    /**
     * Gets username from user input.
     * <p>
     * In a full implementation, this would be a GUI input field.
     *
     * @return the username
     */
    private String getUsernameFromInput() {
        System.out.print("Enter your username: ");
        Scanner scanner = new Scanner(System.in);
        String username = scanner.nextLine().trim();

        if (username.isEmpty()) {
            username = "Player" + System.currentTimeMillis() % 1000;
        }

        return username;
    }
}
