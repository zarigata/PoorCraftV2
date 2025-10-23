package com.poorcraft.client.ui.screens;

import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.Button;
import com.poorcraft.client.ui.widgets.Label;
import com.poorcraft.client.ui.widgets.Panel;

/**
 * Pause screen overlay for in-game pause menu.
 */
public class PauseScreen extends UIScreen {

    public PauseScreen(UIManager uiManager, Configuration config) {
        super(uiManager, config);
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        Panel pausePanel = new Panel(
            screenWidth / 2 - 150,
            screenHeight / 2 - 150,
            300,
            300
        );
        pausePanel.setBackgroundColor(Color.rgba(0, 0, 0, 128));
        pausePanel.setPadding(20);

        Label titleLabel = new Label(0, 0, 260, 40, "Game Paused");
        titleLabel.setAlignment(Label.Alignment.CENTER);
        titleLabel.setColor(Color.WHITE);
        pausePanel.addChild(titleLabel);

        Button resumeButton = new Button(0, 50, 260, 40, "Resume", this::onResumeClick);
        resumeButton.setColors(
            Color.rgba(40, 80, 40, 220),
            Color.rgba(50, 100, 50, 240),
            Color.rgba(30, 60, 30, 240),
            Color.WHITE
        );
        pausePanel.addChild(resumeButton);

        Button settingsButton = new Button(0, 100, 260, 40, "Settings", this::onSettingsClick);
        settingsButton.setColors(
            Color.rgba(60, 60, 60, 220),
            Color.rgba(90, 90, 90, 240),
            Color.rgba(40, 40, 40, 240),
            Color.WHITE
        );
        pausePanel.addChild(settingsButton);

        Button disconnectButton = new Button(0, 150, 260, 40, "Disconnect", this::onDisconnectClick);
        disconnectButton.setColors(
            Color.rgba(80, 40, 40, 220),
            Color.rgba(110, 50, 50, 240),
            Color.rgba(60, 30, 30, 240),
            Color.WHITE
        );
        pausePanel.addChild(disconnectButton);

        setRootWidget(pausePanel);
    }

    @Override
    protected void updateCursorMode() {
        // Show cursor for pause menu
    }

    private void onResumeClick() {
        uiManager.setState(UIState.IN_GAME);
    }

    private void onSettingsClick() {
        uiManager.setState(UIState.SETTINGS);
    }

    private void onDisconnectClick() {
        // TODO: Implement disconnect logic
        uiManager.setState(UIState.MAIN_MENU);
    }
}
