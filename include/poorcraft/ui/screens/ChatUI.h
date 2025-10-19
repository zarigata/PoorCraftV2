#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <cstdint>
#include <deque>
#include <string>

namespace PoorCraft {

class NetworkManager;
class GameStateManager;
using Config = ::poorcraft::Config;

class ChatUI : public UIScreen {
public:
    struct ChatMessage {
        std::string sender;
        std::string message;
        double timestamp;
        bool system;
    };

    ChatUI(NetworkManager& networkManager, Config& config, GameStateManager& stateManager);

    void onEnter() override;
    void update(float deltaTime) override;
    void render() override;

    void toggleChat();
    bool isChatOpen() const;

    void addMessage(const std::string& sender, const std::string& message, bool system = false);

private:
    void renderChatWindow();
    void renderChatOverlay();
    void sendMessage();

    NetworkManager& m_NetworkManager;
    Config& m_Config;
    GameStateManager& m_StateManager;

    std::deque<ChatMessage> m_Messages;
    bool m_ChatOpen = false;
    float m_FadeTime = 10.0f;
    std::size_t m_MaxMessages = 100;

    char m_InputBuffer[256];
};

} // namespace PoorCraft
