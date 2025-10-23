package com.poorcraft.client.ui.screens;

import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.Button;
import com.poorcraft.client.ui.widgets.Label;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.client.ui.widgets.ProgressBar;

/**
 * Connection screen for showing connection progress and status.
 */
public class ConnectionScreen extends UIScreen {

    public ConnectionScreen(UIManager uiManager, Configuration config) {
        super(uiManager, config);
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        Panel connectionPanel = new Panel(
            screenWidth / 2 - 150,
            screenHeight / 2 - 100,
            300,
            200
        );
        connectionPanel.setBackgroundColor(Color.rgba(0, 0, 0, 128));
        connectionPanel.setPadding(20);

        Label titleLabel = new Label(0, 0, 260, 40, "Connecting...");
        titleLabel.setAlignment(Label.Alignment.CENTER);
        titleLabel.setColor(Color.WHITE);
        connectionPanel.addChild(titleLabel);

        // TODO: Implement progress bar, status messages, cancel button

        Label statusLabel = new Label(0, 50, 260, 30, "Connecting to server...");
        statusLabel.setAlignment(Label.Alignment.CENTER);
        statusLabel.setColor(Color.WHITE);
        connectionPanel.addChild(statusLabel);

        ProgressBar progressBar = new ProgressBar(20, 90, 260, 20, 0.0f);
        progressBar.setColor(Color.rgba(0, 120, 255, 255));
        connectionPanel.addChild(progressBar);

        Button cancelButton = new Button(0, 120, 260, 40, "Cancel", this::onCancelClick);
        cancelButton.setColors(
            Color.rgba(80, 40, 40, 220),
            Color.rgba(110, 50, 50, 240),
            Color.rgba(60, 30, 30, 240),
            Color.WHITE
        );
        connectionPanel.addChild(cancelButton);

        setRootWidget(connectionPanel);
    }

    @Override
    protected void updateCursorMode() {
        // Show cursor for connection screen
    }

    private void onCancelClick() {
        // TODO: Implement cancel connection logic
        uiManager.setState(UIState.MAIN_MENU);
    }
}
