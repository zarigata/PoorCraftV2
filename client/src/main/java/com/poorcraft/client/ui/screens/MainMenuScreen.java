package com.poorcraft.client.ui.screens;

import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.Button;
import com.poorcraft.client.ui.widgets.Label;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.common.Constants;

/**
 * Main menu screen with game title and navigation buttons.
 */
public class MainMenuScreen extends UIScreen {
    private Panel mainPanel;
    private Label titleLabel;
    private Button singleplayerButton;
    private Button multiplayerButton;
    private Button settingsButton;
    private Button quitButton;

    public MainMenuScreen(UIManager uiManager, Configuration config) {
        super(uiManager, config);
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        // Create main panel centered on screen
        mainPanel = new Panel(
            screenWidth / 2 - 150,
            screenHeight / 2 - 150,
            300,
            300
        );
        mainPanel.setBackgroundColor(Color.rgba(0, 0, 0, 128));
        mainPanel.setPadding(20);

        // Title label
        titleLabel = new Label(0, 0, 260, 60, Constants.Game.NAME + " v" + Constants.Game.VERSION);
        titleLabel.setAlignment(Label.Alignment.CENTER);
        titleLabel.setColor(Color.WHITE);
        mainPanel.addChild(titleLabel);

        // Singleplayer button
        singleplayerButton = new Button(0, 70, 260, 40, "Singleplayer", this::onSingleplayerClick);
        singleplayerButton.setColors(
            Color.rgba(60, 60, 60, 220),
            Color.rgba(90, 90, 90, 240),
            Color.rgba(40, 40, 40, 240),
            Color.WHITE
        );
        mainPanel.addChild(singleplayerButton);

        // Multiplayer button
        multiplayerButton = new Button(0, 120, 260, 40, "Multiplayer", this::onMultiplayerClick);
        multiplayerButton.setColors(
            Color.rgba(60, 60, 60, 220),
            Color.rgba(90, 90, 90, 240),
            Color.rgba(40, 40, 40, 240),
            Color.WHITE
        );
        mainPanel.addChild(multiplayerButton);

        // Settings button
        settingsButton = new Button(0, 170, 260, 40, "Settings", this::onSettingsClick);
        settingsButton.setColors(
            Color.rgba(60, 60, 60, 220),
            Color.rgba(90, 90, 90, 240),
            Color.rgba(40, 40, 40, 240),
            Color.WHITE
        );
        mainPanel.addChild(settingsButton);

        // Quit button
        quitButton = new Button(0, 220, 260, 40, "Quit", this::onQuitClick);
        quitButton.setColors(
            Color.rgba(80, 40, 40, 220),
            Color.rgba(110, 50, 50, 240),
            Color.rgba(60, 30, 30, 240),
            Color.WHITE
        );
        mainPanel.addChild(quitButton);

        setRootWidget(mainPanel);
    }

    @Override
    protected void updateCursorMode() {
        // Show cursor for main menu
        if (uiManager != null) {
            // This would need window access - for now assume it's handled by UIManager
        }
    }

    private void onSingleplayerClick() {
        // TODO: Implement singleplayer mode
        System.out.println("Singleplayer clicked (not implemented)");
    }

    private void onMultiplayerClick() {
        uiManager.setState(UIState.SERVER_BROWSER);
    }

    private void onSettingsClick() {
        uiManager.setState(UIState.SETTINGS);
    }

    private void onQuitClick() {
        System.exit(0);
    }
}
