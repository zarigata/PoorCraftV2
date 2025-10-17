#include "poorcraft/resource/Resource.h"

namespace PoorCraft {

Resource::Resource(const std::string& path)
    : m_Path(path)
    , m_State(ResourceState::Unloaded)
    , m_Size(0) {
}

} // namespace PoorCraft
