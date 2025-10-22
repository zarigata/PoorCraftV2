package com.poorcraft.client.ui;

import com.poorcraft.common.config.Configuration;
import com.poorcraft.client.network.ClientNetworkManager;
import org.slf4j.Logger;

/**
 * Simple server browser UI for multiplayer mode.
 * <p>
 * In a full implementation, this would show a list of servers,
 * allow adding/removing servers, and handle server selection.
 * For now, it's a placeholder that returns a hardcoded server.
 */
public class ServerBrowserUI {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ServerBrowserUI.class);

    private final Configuration config;
    private final ClientNetworkManager networkManager;

    /**
     * Creates a new server browser UI.
     *
     * @param config the client configuration
     * @param networkManager the network manager
     */
    public ServerBrowserUI(Configuration config, ClientNetworkManager networkManager) {
        this.config = config;
        this.networkManager = networkManager;
    }

    /**
     * Shows the server browser and returns the selected server.
     * <p>
     * For now, this returns a hardcoded localhost server.
     * In a full implementation, this would show a GUI with server list,
     * ping times, player counts, etc.
     *
     * @return the selected server address, or null if cancelled
     */
    public String showAndGetSelection() {
        LOGGER.info("Showing server browser UI...");

        // TODO: Implement proper server browser GUI
        // For now, just return the default server from config

        String defaultServer = config.getString("multiplayer.defaultServerAddress", "localhost");
        int defaultPort = config.getInt("multiplayer.defaultServerPort", 25565);

        String serverAddress = defaultServer + ":" + defaultPort;

        LOGGER.info("Selected server: {}", serverAddress);
        return serverAddress;
    }
}
