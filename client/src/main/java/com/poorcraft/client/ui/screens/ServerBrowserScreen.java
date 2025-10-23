package com.poorcraft.client.ui.screens;

import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.Button;
import com.poorcraft.client.ui.widgets.Label;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.client.ui.widgets.TextField;

import java.util.ArrayList;
import java.util.List;

/**
 * Server browser screen for multiplayer connection.
 */
public class ServerBrowserScreen extends UIScreen {
    private final ClientNetworkManager networkManager;

    private Panel mainPanel;
    private Label titleLabel;
    private TextField addressField;
    private Button connectButton;
    private Button refreshButton;
    private Button backButton;

    // Simple server list for demo (would be populated from server list in real implementation)
    private final List<String> servers = new ArrayList<>();
    private int selectedServerIndex = -1;

    public ServerBrowserScreen(UIManager uiManager, Configuration config, ClientNetworkManager networkManager) {
        super(uiManager, config);
        this.networkManager = networkManager;

        // Add some demo servers
        servers.add("localhost:25565");
        servers.add("demo.poorcraft.com:25565");
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        // Create main panel
        mainPanel = new Panel(
            screenWidth / 2 - 200,
            screenHeight / 2 - 200,
            400,
            400
        );
        mainPanel.setBackgroundColor(Color.rgba(0, 0, 0, 128));
        mainPanel.setPadding(20);

        // Title
        titleLabel = new Label(0, 0, 360, 40, "Server Browser");
        titleLabel.setAlignment(Label.Alignment.CENTER);
        titleLabel.setColor(Color.WHITE);
        mainPanel.addChild(titleLabel);

        // Address field
        addressField = new TextField(0, 50, 360, 30, "Enter server address");
        addressField.setPlaceholder("localhost:25565");
        mainPanel.addChild(addressField);

        // Connect button
        connectButton = new Button(0, 90, 360, 40, "Connect", this::onConnectClick);
        connectButton.setColors(
            Color.rgba(40, 80, 40, 220),
            Color.rgba(50, 100, 50, 240),
            Color.rgba(30, 60, 30, 240),
            Color.WHITE
        );
        mainPanel.addChild(connectButton);

        // Server list area (simplified for now)
        Label serverListLabel = new Label(0, 150, 360, 30, "Available Servers:");
        serverListLabel.setColor(Color.WHITE);
        mainPanel.addChild(serverListLabel);

        // Demo server buttons
        for (int i = 0; i < Math.min(servers.size(), 5); i++) {
            String server = servers.get(i);
            Button serverButton = new Button(0, 190 + i * 45, 360, 40, server, () -> onServerSelect(i));
            serverButton.setColors(
                Color.rgba(50, 50, 80, 220),
                Color.rgba(70, 70, 100, 240),
                Color.rgba(40, 40, 60, 240),
                Color.WHITE
            );
            mainPanel.addChild(serverButton);
        }

        // Bottom buttons
        refreshButton = new Button(0, 350, 180, 40, "Refresh", this::onRefreshClick);
        refreshButton.setColors(
            Color.rgba(60, 60, 60, 220),
            Color.rgba(90, 90, 90, 240),
            Color.rgba(40, 40, 40, 240),
            Color.WHITE
        );
        mainPanel.addChild(refreshButton);

        backButton = new Button(200, 350, 180, 40, "Back", this::onBackClick);
        backButton.setColors(
            Color.rgba(80, 40, 40, 220),
            Color.rgba(110, 50, 50, 240),
            Color.rgba(60, 30, 30, 240),
            Color.WHITE
        );
        mainPanel.addChild(backButton);

        setRootWidget(mainPanel);
    }

    private void onConnectClick() {
        String address = addressField.getText();
        if (!address.isEmpty()) {
            // TODO: Implement actual connection logic
            System.out.println("Connecting to: " + address);
            uiManager.setState(UIState.CONNECTING);
        }
    }

    private void onServerSelect(int index) {
        addressField.setText(servers.get(index));
    }

    private void onRefreshClick() {
        // TODO: Implement server list refresh
        System.out.println("Refreshing server list...");
    }

    private void onBackClick() {
        uiManager.setState(UIState.MAIN_MENU);
    }
}
