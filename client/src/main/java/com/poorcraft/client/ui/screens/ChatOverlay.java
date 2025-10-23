package com.poorcraft.client.ui.screens;

import com.poorcraft.client.network.ClientNetworkManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.UIManager;
import com.poorcraft.client.ui.UIState;
import com.poorcraft.client.ui.UIScreen;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.client.ui.layout.Anchor;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.client.ui.widgets.TextField;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.common.network.packet.ChatMessagePacket;
import org.lwjgl.glfw.GLFW;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.Iterator;
import java.util.List;

/**
 * Chat overlay for in-game messaging and commands.
 */
public class ChatOverlay extends UIScreen {
    private static final int MAX_MESSAGES = 100;
    private static final int MAX_VISIBLE_MESSAGES = 20;
    private static final float DEFAULT_FADE_SECONDS = 8.0f;
    private static final float FADE_WINDOW_SECONDS = 2.5f;

    private final Deque<Message> messageQueue = new ArrayDeque<>(MAX_MESSAGES);
    private final float configuredOpacity;
    private final float fadeDuration;
    private ClientNetworkManager networkManager;

    private Panel rootPanel;
    private TextField inputField;
    private ChatHistoryWidget historyWidget;
    private boolean active;

    public ChatOverlay(UIManager uiManager, Configuration config) {
        super(uiManager, config);
        this.configuredOpacity = clampOpacity(config.getFloat("ui.chatOpacity", 0.75f));
        this.fadeDuration = Math.max(1.0f, config.getFloat("ui.chatFadeSeconds", DEFAULT_FADE_SECONDS));
    }

    public void setNetworkManager(ClientNetworkManager networkManager) {
        this.networkManager = networkManager;
    }

    @Override
    protected void buildUI() {
        int screenWidth = getScreenWidth();
        int screenHeight = getScreenHeight();

        rootPanel = new Panel(0, 0, screenWidth, screenHeight);
        rootPanel.setBackgroundColor(Color.TRANSPARENT);

        historyWidget = new ChatHistoryWidget();
        historyWidget.setAnchor(Anchor.BOTTOM_LEFT, 16.0f, -80.0f);
        historyWidget.setSize(screenWidth - 32.0f, screenHeight - 160.0f);
        rootPanel.addChild(historyWidget);

        inputField = new TextField(0, 0, screenWidth - 32.0f, 28.0f, "Type a message...");
        inputField.setAnchor(Anchor.BOTTOM_LEFT, 16.0f, -36.0f);
        inputField.setVisible(false);
        inputField.setEnabled(false);
        inputField.setOnSubmit(this::sendMessage);
        rootPanel.addChild(inputField);

        setRootWidget(rootPanel);
    }

    @Override
    public void resize(int width, int height) {
        super.resize(width, height);

        if (rootPanel != null) {
            rootPanel.setSize(width, height);
        }
        if (historyWidget != null) {
            historyWidget.setSize(Math.max(100.0f, width - 32.0f), Math.max(100.0f, height - 160.0f));
        }
        if (inputField != null) {
            inputField.setSize(Math.max(100.0f, width - 32.0f), inputField.getHeight());
            inputField.setAnchor(Anchor.BOTTOM_LEFT, 16.0f, -36.0f);
        }
    }

    @Override
    public void onShow() {
        super.onShow();
        activate();
    }

    @Override
    public void onHide() {
        super.onHide();
        deactivate();
    }

    @Override
    public void update(float dt) {
        super.update(dt);
        historyWidget.setActive(active);
    }

    @Override
    public boolean handleInput(com.poorcraft.client.input.InputManager input) {
        boolean handled = super.handleInput(input);

        if (!active) {
            return handled;
        }

        if (input.keyPressed(GLFW.GLFW_KEY_ESCAPE)) {
            cancel();
            return true;
        }

        return true;
    }

    @Override
    protected boolean shouldConsumeInput() {
        return active;
    }

    @Override
    protected void updateCursorMode() {
        if (uiManager != null) {
            uiManager.setCursorMode(GLFW.GLFW_CURSOR_NORMAL);
        }
    }

    public void addMessage(String text, int type) {
        if (text == null || text.isEmpty()) {
            return;
        }

        synchronized (messageQueue) {
            if (messageQueue.size() >= MAX_MESSAGES) {
                messageQueue.removeFirst();
            }
            messageQueue.addLast(new Message(text, type, System.nanoTime()));
        }

        if (historyWidget != null) {
            historyWidget.invalidate();
        }
    }

    private void activate() {
        active = true;
        if (inputField != null) {
            inputField.setVisible(true);
            inputField.setEnabled(true);
            inputField.setFocused(true);
        }
        if (historyWidget != null) {
            historyWidget.invalidate();
        }
    }

    private void deactivate() {
        active = false;
        if (inputField != null) {
            inputField.clear();
            inputField.setFocused(false);
            inputField.setVisible(false);
            inputField.setEnabled(false);
        }
        if (historyWidget != null) {
            historyWidget.invalidate();
        }
    }

    private void sendMessage(String message) {
        if (message == null) {
            message = "";
        }

        String trimmed = message.trim();
        if (!trimmed.isEmpty()) {
            if (networkManager != null && networkManager.isConnected()) {
                networkManager.sendPacket(new ChatMessagePacket(trimmed, 0));
            } else {
                addMessage(trimmed, 0);
            }
        }

        if (uiManager != null) {
            uiManager.setState(UIState.IN_GAME);
        }
    }

    private void cancel() {
        if (uiManager != null) {
            uiManager.setState(UIState.IN_GAME);
        }
    }

    private float clampOpacity(float opacity) {
        return Math.max(0.05f, Math.min(1.0f, opacity));
    }

    private final class ChatHistoryWidget extends Widget {
        private boolean cacheDirty = true;
        private List<Message> snapshot = List.of();
        private boolean activeInput;

        private ChatHistoryWidget() {
            super(0, 0, 0, 0);
        }

        @Override
        public void setSize(float width, float height) {
            super.setSize(width, height);
            cacheDirty = true;
        }

        void invalidate() {
            cacheDirty = true;
        }

        void setActive(boolean active) {
            this.activeInput = active;
        }

        @Override
        public void render(UIRenderer renderer, float mouseX, float mouseY) {
            if (!isVisible()) {
                return;
            }

            if (cacheDirty) {
                snapshot = copyMessages();
                cacheDirty = false;
            }

            if (snapshot.isEmpty()) {
                return;
            }

            float lineHeight = renderer.getLineHeight() + 2.0f;
            float baseX = getX();
            float baseY = getY() + getHeight();

            long now = System.nanoTime();
            int rendered = 0;

            for (int i = snapshot.size() - 1; i >= 0 && rendered < MAX_VISIBLE_MESSAGES; i--) {
                Message message = snapshot.get(i);
                float ageSeconds = (now - message.timestamp) / 1_000_000_000.0f;
                float alphaMultiplier = activeInput ? 1.0f : computeAlpha(ageSeconds);
                if (alphaMultiplier <= 0.01f) {
                    continue;
                }

                String text = message.text;
                float textWidth = renderer.measureText(text);
                float x = baseX;
                float y = baseY - lineHeight * (rendered + 1);

                int backgroundAlpha = (int) (alphaMultiplier * configuredOpacity * 180.0f);
                int textAlpha = (int) (alphaMultiplier * configuredOpacity * 255.0f);

                if (backgroundAlpha > 0) {
                    int backgroundColor = Color.rgba(0, 0, 0, backgroundAlpha);
                    renderer.drawRect(x - 6.0f, y - (lineHeight - renderer.getLineHeight()),
                        textWidth + 12.0f, lineHeight, backgroundColor);
                }

                int textColor = Color.withAlpha(Color.WHITE, textAlpha);
                renderer.drawText(text, x, y, textColor);
                rendered++;
            }
        }

        @Override
        public boolean handleInput(com.poorcraft.client.input.InputManager input, float mouseX, float mouseY) {
            return false;
        }

        private List<Message> copyMessages() {
            List<Message> copy = new ArrayList<>(messageQueue.size());
            synchronized (messageQueue) {
                Iterator<Message> iterator = messageQueue.iterator();
                while (iterator.hasNext()) {
                    copy.add(iterator.next());
                }
            }
            return copy;
        }

        private float computeAlpha(float ageSeconds) {
            if (ageSeconds >= fadeDuration) {
                return 0.0f;
            }
            if (ageSeconds <= fadeDuration - FADE_WINDOW_SECONDS) {
                return 1.0f;
            }
            float fadeProgress = (ageSeconds - (fadeDuration - FADE_WINDOW_SECONDS)) / FADE_WINDOW_SECONDS;
            return Math.max(0.0f, 1.0f - fadeProgress);
        }
    }

    private record Message(String text, int type, long timestamp) {}
}
