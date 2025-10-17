#include "poorcraft/resource/BinaryResource.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"

namespace PoorCraft {

BinaryResource::BinaryResource(const std::string& path)
    : Resource(path) {
}

bool BinaryResource::load() {
    std::vector<uint8_t> data;
    auto result = poorcraft::Platform::read_file_binary(m_Path, data);

    if (result != poorcraft::Platform::FileOperationResult::Success) {
        PC_ERROR("[BinaryResource] Failed to load binary file: " + m_Path + " (" +
            poorcraft::Platform::file_operation_result_to_string(result) + ")");
        setSize(0);
        m_Data.clear();
        return false;
    }

    m_Data = std::move(data);
    setSize(m_Data.size());

    PC_INFO("[BinaryResource] Loaded binary file: " + m_Path + " (" +
        std::to_string(getSize()) + " bytes)");

    return true;
}

void BinaryResource::unload() {
    m_Data.clear();
    setSize(0);
    PC_INFO("[BinaryResource] Unloaded binary file: " + m_Path);
}

} // namespace PoorCraft
