package com.poorcraft.client.ui;

public enum UIState {
    MAIN_MENU("main_menu"),
    SERVER_BROWSER("server_browser"),
    CONNECTING("connecting"),
    IN_GAME("in_game"),
    INVENTORY("inventory"),
    SETTINGS("settings"),
    PAUSED("paused"),
    CHAT("chat");

    private final String id;

    UIState(String id) {
        this.id = id;
    }

    public String getId() {
        return id;
    }
}
