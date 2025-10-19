#pragma once

namespace PoorCraft {

class UIScreen {
public:
    UIScreen() = default;
    virtual ~UIScreen() = default;

    virtual void onEnter();
    virtual void onExit();

    virtual void update(float /*deltaTime*/) {}
    virtual void render() = 0;

    bool isActive() const;
    void setActive(bool active);

private:
    bool m_Active = false;
};

} // namespace PoorCraft
