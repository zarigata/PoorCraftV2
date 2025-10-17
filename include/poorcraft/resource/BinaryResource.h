#pragma once

#include "poorcraft/resource/Resource.h"
#include <vector>
#include <cstdint>

namespace PoorCraft {

class BinaryResource : public Resource {
public:
    explicit BinaryResource(const std::string& path);
    ~BinaryResource() override = default;

    // Resource interface
    bool load() override;
    void unload() override;
    ResourceType getType() const override { return ResourceType::Binary; }

    // Data access
    const std::vector<uint8_t>& getData() const { return m_Data; }
    const uint8_t* getDataPtr() const { return m_Data.empty() ? nullptr : m_Data.data(); }

private:
    std::vector<uint8_t> m_Data;
};

} // namespace PoorCraft
