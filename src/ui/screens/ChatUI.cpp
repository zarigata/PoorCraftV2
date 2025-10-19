#include "poorcraft/ui/screens/ChatUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/ui/GameState.h"

#include <imgui.h>
#include <chrono>
#include <cstring>

namespace PoorCraft {

ChatUI::ChatUI(NetworkManager& networkManager, Config& config, GameStateManager& stateManager)
    : m_NetworkManager(networkManager), m_Config(config), m_StateManager(stateManager) {
    std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
    m_MaxMessages = static_cast<std::size_t>(m_Config.get_int(Config::UIConfig::CHAT_MAX_MESSAGES_KEY, 100));
    m_FadeTime = m_Config.get_float(Config::UIConfig::CHAT_FADE_TIME_KEY, 10.0f);
}

void ChatUI::onEnter() {
    UIScreen::onEnter();
    m_ChatOpen = false;
}

void ChatUI::update(float deltaTime) {
    // Prune messages beyond max
    while (m_Messages.size() > m_MaxMessages) {
        m_Messages.pop_front();
    }

    // Age messages for fade overlay
    auto now = std::chrono::steady_clock::now();
    double currentTime = std::chrono::duration<double>(now.time_since_epoch()).count();
    
    // Update timestamps for fading
    for (auto& msg : m_Messages) {
        // Messages fade based on time elapsed since timestamp
        (void)msg; // Timestamp is already stored, fade calculation happens in render
    }
}

void ChatUI::render() {
    if (!isActive()) {
        return;
    }

    if (m_ChatOpen) {
        renderChatWindow();
    } else {
        renderChatOverlay();
    }
}

void ChatUI::renderChatWindow() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 20.0f, viewport->Pos.y + viewport->Size.y - 400.0f));
    ImGui::SetNextWindowSize(ImVec2(600.0f, 350.0f));
    ImGui::SetNextWindowBgAlpha(0.9f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | 
                                             ImGuiWindowFlags_NoResize | 
                                             ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("Chat", nullptr, windowFlags)) {
        // Scrollable message history
        ImGui::BeginChild("ChatHistory", ImVec2(0, -30), true);
        
        for (const auto& msg : m_Messages) {
            if (msg.system) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::TextWrapped("[SYSTEM] %s", msg.message.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                if (!msg.sender.empty()) {
                    ImGui::TextWrapped("<%s> %s", msg.sender.c_str(), msg.message.c_str());
                } else {
                    ImGui::TextWrapped("%s", msg.message.c_str());
                }
                ImGui::PopStyleColor();
            }
        }

        // Auto-scroll to bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();

        // Input field
        ImGui::SetNextItemWidth(-70.0f);
        bool enterPressed = ImGui::InputText("##ChatInput", m_InputBuffer, sizeof(m_InputBuffer), 
                                             ImGuiInputTextFlags_EnterReturnsTrue);
        
        ImGui::SameLine();
        bool sendClicked = ImGui::Button("Send", ImVec2(60.0f, 0.0f));

        if (enterPressed || sendClicked) {
            sendMessage();
        }
    }
    ImGui::End();
}

void ChatUI::renderChatOverlay() {
    // Draw last 5 messages faded in bottom-left corner
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* drawList = ImGui::GetForegroundDrawList(viewport);

    auto now = std::chrono::steady_clock::now();
    double currentTime = std::chrono::duration<double>(now.time_since_epoch()).count();

    float yOffset = viewport->Pos.y + viewport->Size.y - 120.0f;
    int displayCount = 0;
    const int maxDisplay = 5;

    // Display most recent messages
    for (auto it = m_Messages.rbegin(); it != m_Messages.rend() && displayCount < maxDisplay; ++it) {
        double age = currentTime - it->timestamp;
        if (age > m_FadeTime) {
            continue; // Skip messages older than fade time
        }

        float alpha = 1.0f - static_cast<float>(age / m_FadeTime);
        ImVec4 color = it->system ? ImVec4(0.7f, 0.7f, 0.7f, alpha) : ImVec4(1.0f, 1.0f, 1.0f, alpha);

        std::string displayText;
        if (it->system) {
            displayText = "[SYSTEM] " + it->message;
        } else if (!it->sender.empty()) {
            displayText = "<" + it->sender + "> " + it->message;
        } else {
            displayText = it->message;
        }

        drawList->AddText(ImVec2(viewport->Pos.x + 20.0f, yOffset), 
                         ImGui::GetColorU32(color), 
                         displayText.c_str());
        
        yOffset -= 20.0f;
        displayCount++;
    }
}

void ChatUI::toggleChat() {
    m_ChatOpen = !m_ChatOpen;
    if (m_ChatOpen) {
        std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
    }
}

bool ChatUI::isChatOpen() const {
    return m_ChatOpen;
}

void ChatUI::addMessage(const std::string& sender, const std::string& message, bool system) {
    auto now = std::chrono::steady_clock::now();
    double timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();

    ChatMessage msg;
    msg.sender = sender;
    msg.message = message;
    msg.timestamp = timestamp;
    msg.system = system;

    m_Messages.push_back(msg);

    // Enforce max size
    while (m_Messages.size() > m_MaxMessages) {
        m_Messages.pop_front();
    }

    PC_INFO("ChatUI: Message added from " + (sender.empty() ? "SYSTEM" : sender));
}

void ChatUI::sendMessage() {
    std::string message(m_InputBuffer);
    
    // Trim whitespace
    size_t start = message.find_first_not_of(" \t\n\r");
    size_t end = message.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos || end == std::string::npos) {
        // Empty message
        std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
        return;
    }
    
    message = message.substr(start, end - start + 1);

    if (message.empty()) {
        std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
        return;
    }

    // Send message
    if (m_NetworkManager.isClient()) {
        // Forward to network in multiplayer
        PC_INFO("ChatUI: Sending message to server: " + message);
        // TODO: Implement NetworkClient::sendChatMessage when available
        addMessage("You", message, false);
    } else {
        // Local message in singleplayer
        addMessage("You", message, false);
    }

    // Clear input buffer
    std::memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
}

} // namespace PoorCraft
