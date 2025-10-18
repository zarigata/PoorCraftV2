#pragma once

#include "poorcraft/world/BlockType.h"

#include "poorcraft/core/Logger.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace PoorCraft {

class BlockRegistry {
public:
    static BlockRegistry& getInstance();

    void initialize();
    void clear();

    uint16_t registerBlock(BlockType block);

    [[nodiscard]] const BlockType& getBlock(uint16_t id) const;
    [[nodiscard]] const BlockType* getBlockByName(const std::string& name) const;
    [[nodiscard]] uint16_t getBlockID(const std::string& name) const;

    [[nodiscard]] bool hasBlock(uint16_t id) const;
    [[nodiscard]] bool hasBlock(const std::string& name) const;

    [[nodiscard]] size_t getBlockCount() const;

private:
    BlockRegistry();

    BlockType airBlock;
    mutable std::mutex mutex;
    std::unordered_map<uint16_t, BlockType> blocks;
    std::unordered_map<std::string, uint16_t> nameToId;
    uint16_t nextId;
};

} // namespace PoorCraft
