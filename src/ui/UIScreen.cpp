#include "poorcraft/ui/UIScreen.h"

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

void UIScreen::onEnter() {
    m_Active = true;
    PC_DEBUG("UIScreen entered");
}

void UIScreen::onExit() {
    m_Active = false;
    PC_DEBUG("UIScreen exited");
}

bool UIScreen::isActive() const {
    return m_Active;
}

void UIScreen::setActive(bool active) {
    m_Active = active;
}

} // namespace PoorCraft
