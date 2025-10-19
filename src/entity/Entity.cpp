#include "poorcraft/entity/Entity.h"

#include <utility>

namespace PoorCraft {

Entity::Entity(EntityID id, std::string name)
    : m_Id(id),
      m_Name(std::move(name)) {
    PC_INFO("Entity created: " + m_Name + " (" + std::to_string(m_Id) + ")");
}

EntityID Entity::getId() const {
    return m_Id;
}

const std::string& Entity::getName() const {
    return m_Name;
}

bool Entity::isActive() const {
    return m_Active;
}

void Entity::setActive(bool active) {
    m_Active = active;
    PC_TRACE("Entity '" + m_Name + "' set active: " + (m_Active ? "true" : "false"));
}

} // namespace PoorCraft
