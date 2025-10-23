package com.poorcraft.client.ui.screens;

import com.poorcraft.client.ui.*;
import com.poorcraft.client.ui.widgets.*;

/**
 * Settings screen with configuration options organized in tabs.
 */
public class SettingsScreen extends UIScreen {
    private Panel mainPanel;
    private TabPanel tabPanel;
    private Button applyButton;
    private Button cancelButton;

    public SettingsScreen(UIManager uiManager, Configuration config) {
        super(uiManager, config);
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        // Create main panel
        mainPanel = new Panel(
            screenWidth / 2 - 250,
            screenHeight / 2 - 225,
            500,
            450
        );
        mainPanel.setBackgroundColor(Color.rgba(0, 0, 0, 128));
        mainPanel.setPadding(20);

        // Title
        Label titleLabel = new Label(0, 0, 460, 40, "Settings");
        titleLabel.setAlignment(Label.Alignment.CENTER);
        titleLabel.setColor(Color.WHITE);
        mainPanel.addChild(titleLabel);

        // Tab panel for different settings categories
        tabPanel = new TabPanel(0, 50, 460, 300);
        mainPanel.addChild(tabPanel);

        // Graphics tab
        Panel graphicsTab = createGraphicsTab();
        tabPanel.addTab("Graphics", graphicsTab);

        // Controls tab
        Panel controlsTab = createControlsTab();
        tabPanel.addTab("Controls", controlsTab);

        // Audio tab
        Panel audioTab = createAudioTab();
        tabPanel.addTab("Audio", audioTab);

        // Gameplay tab
        Panel gameplayTab = createGameplayTab();
        tabPanel.addTab("Gameplay", gameplayTab);

        // Bottom buttons
        applyButton = new Button(0, 360, 220, 40, "Apply", this::onApplyClick);
        applyButton.setColors(
            Color.rgba(40, 80, 40, 220),
            Color.rgba(50, 100, 50, 240),
            Color.rgba(30, 60, 30, 240),
            Color.WHITE
        );
        mainPanel.addChild(applyButton);

        cancelButton = new Button(240, 360, 220, 40, "Cancel", this::onCancelClick);
        cancelButton.setColors(
            Color.rgba(80, 40, 40, 220),
            Color.rgba(110, 50, 50, 240),
            Color.rgba(60, 30, 30, 240),
            Color.WHITE
        );
        mainPanel.addChild(cancelButton);

        setRootWidget(mainPanel);
    }

    private Panel createGraphicsTab() {
        Panel tab = new Panel(0, 0, 400, 250);
        tab.setPadding(10);

        // UI Scale slider
        Label uiScaleLabel = new Label(0, 0, 200, 30, "UI Scale:");
        uiScaleLabel.setColor(Color.WHITE);
        tab.addChild(uiScaleLabel);

        Slider uiScaleSlider = new Slider(0, 35, 380, 20, 0.5f, 2.0f, 1.0f);
        uiScaleSlider.setOnChange(value -> {
            config.setFloat("ui.scale", value);
        });
        tab.addChild(uiScaleSlider);

        // FOV slider
        Label fovLabel = new Label(0, 70, 200, 30, "Field of View:");
        fovLabel.setColor(Color.WHITE);
        tab.addChild(fovLabel);

        Slider fovSlider = new Slider(0, 105, 380, 20, 60.0f, 120.0f, 90.0f);
        fovSlider.setOnChange(value -> {
            config.setFloat("game.fov", value);
        });
        tab.addChild(fovSlider);

        // Render distance slider
        Label renderDistanceLabel = new Label(0, 140, 200, 30, "Render Distance:");
        renderDistanceLabel.setColor(Color.WHITE);
        tab.addChild(renderDistanceLabel);

        Slider renderDistanceSlider = new Slider(0, 175, 380, 20, 2, 32, 16);
        renderDistanceSlider.setOnChange(value -> {
            config.setInt("game.renderDistance", (int) value);
        });
        tab.addChild(renderDistanceSlider);

        // VSync checkbox
        Checkbox vsyncCheckbox = new Checkbox(0, 210, 20, 20, "Enable VSync");
        vsyncCheckbox.setOnChange(checked -> {
            config.setBoolean("graphics.vsync", checked);
        });
        tab.addChild(vsyncCheckbox);

        Label vsyncLabel = new Label(25, 205, 200, 30, "Enable VSync");
        vsyncLabel.setColor(Color.WHITE);
        tab.addChild(vsyncLabel);

        return tab;
    }

    private Panel createControlsTab() {
        Panel tab = new Panel(0, 0, 400, 250);
        tab.setPadding(10);

        // Mouse sensitivity slider
        Label sensitivityLabel = new Label(0, 0, 200, 30, "Mouse Sensitivity:");
        sensitivityLabel.setColor(Color.WHITE);
        tab.addChild(sensitivityLabel);

        Slider sensitivitySlider = new Slider(0, 35, 380, 20, 0.1f, 1.0f, 0.15f);
        sensitivitySlider.setOnChange(value -> {
            config.setFloat("input.mouseSensitivity", value);
        });
        tab.addChild(sensitivitySlider);

        // Raw mouse motion checkbox
        Checkbox rawMouseCheckbox = new Checkbox(0, 70, 20, 20, "Raw Mouse Motion");
        rawMouseCheckbox.setOnChange(checked -> {
            config.setBoolean("input.rawMouseMotion", checked);
        });
        tab.addChild(rawMouseCheckbox);

        Label rawMouseLabel = new Label(25, 65, 200, 30, "Raw Mouse Motion");
        rawMouseLabel.setColor(Color.WHITE);
        tab.addChild(rawMouseLabel);

        // Key bindings (simplified)
        Label keybindsLabel = new Label(0, 100, 200, 30, "Key Bindings:");
        keybindsLabel.setColor(Color.WHITE);
        tab.addChild(keybindsLabel);

        KeybindButton forwardButton = new KeybindButton(0, 135, 180, 30, "Forward", "W");
        forwardButton.setOnChange(key -> {
            config.setString("input.key.forward", key);
        });
        tab.addChild(forwardButton);

        KeybindButton jumpButton = new KeybindButton(200, 135, 180, 30, "Jump", "Space");
        jumpButton.setOnChange(key -> {
            config.setString("input.key.jump", key);
        });
        tab.addChild(jumpButton);

        return tab;
    }

    private Panel createAudioTab() {
        Panel tab = new Panel(0, 0, 400, 250);
        tab.setPadding(10);

        // Master volume slider
        Label masterVolumeLabel = new Label(0, 0, 200, 30, "Master Volume:");
        masterVolumeLabel.setColor(Color.WHITE);
        tab.addChild(masterVolumeLabel);

        Slider masterVolumeSlider = new Slider(0, 35, 380, 20, 0.0f, 1.0f, 0.8f);
        masterVolumeSlider.setOnChange(value -> {
            config.setFloat("audio.masterVolume", value);
        });
        tab.addChild(masterVolumeSlider);

        // Music volume slider
        Label musicVolumeLabel = new Label(0, 70, 200, 30, "Music Volume:");
        musicVolumeLabel.setColor(Color.WHITE);
        tab.addChild(musicVolumeLabel);

        Slider musicVolumeSlider = new Slider(0, 105, 380, 20, 0.0f, 1.0f, 0.6f);
        musicVolumeSlider.setOnChange(value -> {
            config.setFloat("audio.musicVolume", value);
        });
        tab.addChild(musicVolumeSlider);

        // Sound effects volume slider
        Label sfxVolumeLabel = new Label(0, 140, 200, 30, "Sound Effects:");
        sfxVolumeLabel.setColor(Color.WHITE);
        tab.addChild(sfxVolumeLabel);

        Slider sfxVolumeSlider = new Slider(0, 175, 380, 20, 0.0f, 1.0f, 0.9f);
        sfxVolumeSlider.setOnChange(value -> {
            config.setFloat("audio.sfxVolume", value);
        });
        tab.addChild(sfxVolumeSlider);

        return tab;
    }

    private Panel createGameplayTab() {
        Panel tab = new Panel(0, 0, 400, 250);
        tab.setPadding(10);

        // Auto-save checkbox
        Checkbox autoSaveCheckbox = new Checkbox(0, 0, 20, 20, "Auto-save");
        autoSaveCheckbox.setOnChange(checked -> {
            config.setBoolean("game.autoSave", checked);
        });
        tab.addChild(autoSaveCheckbox);

        Label autoSaveLabel = new Label(25, -5, 200, 30, "Auto-save");
        autoSaveLabel.setColor(Color.WHITE);
        tab.addChild(autoSaveLabel);

        // Show debug info checkbox
        Checkbox debugInfoCheckbox = new Checkbox(0, 35, 20, 20, "Show Debug Info");
        debugInfoCheckbox.setOnChange(checked -> {
            config.setBoolean("game.showDebugInfo", checked);
        });
        tab.addChild(debugInfoCheckbox);

        Label debugInfoLabel = new Label(25, 30, 200, 30, "Show Debug Info (F3)");
        debugInfoLabel.setColor(Color.WHITE);
        tab.addChild(debugInfoLabel);

        // Difficulty dropdown
        Label difficultyLabel = new Label(0, 70, 200, 30, "Difficulty:");
        difficultyLabel.setColor(Color.WHITE);
        tab.addChild(difficultyLabel);

        Dropdown difficultyDropdown = new Dropdown(0, 105, 200, 30, new String[]{"Peaceful", "Easy", "Normal", "Hard"});
        difficultyDropdown.setOnChange(index -> {
            config.setInt("game.difficulty", index);
        });
        tab.addChild(difficultyDropdown);

        return tab;
    }

    private void onApplyClick() {
        // TODO: Apply settings and save to config
        System.out.println("Applying settings...");
        uiManager.setState(UIState.MAIN_MENU);
    }

    private void onCancelClick() {
        // TODO: Revert settings changes
        System.out.println("Cancelling settings...");
        uiManager.setState(UIState.MAIN_MENU);
    }
}
