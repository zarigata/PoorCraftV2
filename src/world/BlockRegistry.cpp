#include "poorcraft/world/BlockRegistry.h"

#include <utility>

namespace PoorCraft {

BlockRegistry& BlockRegistry::getInstance() {
    static BlockRegistry instance;
    return instance;
}

BlockRegistry::BlockRegistry()
    : airBlock(), mutex(), blocks(), nameToId(), nextId(1) {
    airBlock.setId(0)
        .setName("air")
        .setSolid(false)
        .setOpaque(false)
        .setTransparent(true)
        .setTextureAllFaces("")
        .setLightEmission(0)
        .setHardness(0.0f);

    blocks.emplace(0, airBlock);
    nameToId.emplace("air", 0);
}

void BlockRegistry::initialize() {
    {
        std::scoped_lock lock(mutex);

        PC_INFO("Initializing BlockRegistry with default blocks...");

        blocks.clear();
        nameToId.clear();
        nextId = 1;

        blocks.emplace(0, airBlock);
        nameToId.emplace("air", 0);
    }

    registerBlock(BlockType()
                      .setName("stone")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("stone")
                      .setHardness(2.0f));

    registerBlock(BlockType()
                      .setName("dirt")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("dirt")
                      .setHardness(1.0f));

    registerBlock(BlockType()
                      .setName("grass")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("grass_side")
                      .setTextureForFace(BlockFace::TOP, "grass_top")
                      .setTextureForFace(BlockFace::BOTTOM, "dirt")
                      .setHardness(1.5f));

    registerBlock(BlockType()
                      .setName("sand")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("sand")
                      .setHardness(0.5f));

    registerBlock(BlockType()
                      .setName("water")
                      .setSolid(false)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("water")
                      .setHardness(100.0f));

    registerBlock(BlockType()
                      .setName("snow")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("snow")
                      .setHardness(0.2f));

    registerBlock(BlockType()
                      .setName("ice")
                      .setSolid(true)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("ice")
                      .setHardness(0.5f));

    registerBlock(BlockType()
                      .setName("snow_grass")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTexturePerFace({"grass_side_snowy", "grass_side_snowy", "grass_side_snowy", "grass_side_snowy", "snow", "dirt"})
                      .setHardness(1.5f));

    registerBlock(BlockType()
                      .setName("oak_log")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTexturePerFace({"oak_log_side", "oak_log_side", "oak_log_side", "oak_log_side", "oak_log_top", "oak_log_top"})
                      .setHardness(2.0f));

    registerBlock(BlockType()
                      .setName("oak_leaves")
                      .setSolid(true)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("oak_leaves")
                      .setHardness(0.2f));

    registerBlock(BlockType()
                      .setName("jungle_log")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTexturePerFace({"jungle_log_side", "jungle_log_side", "jungle_log_side", "jungle_log_side", "jungle_log_top", "jungle_log_top"})
                      .setHardness(2.0f));

    registerBlock(BlockType()
                      .setName("jungle_leaves")
                      .setSolid(true)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("jungle_leaves")
                      .setHardness(0.2f));

    registerBlock(BlockType()
                      .setName("spruce_log")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTexturePerFace({"spruce_log_side", "spruce_log_side", "spruce_log_side", "spruce_log_side", "spruce_log_top", "spruce_log_top"})
                      .setHardness(2.0f));

    registerBlock(BlockType()
                      .setName("spruce_leaves")
                      .setSolid(true)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("spruce_leaves")
                      .setHardness(0.2f));

    registerBlock(BlockType()
                      .setName("cactus")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTexturePerFace({"cactus_side", "cactus_side", "cactus_side", "cactus_side", "cactus_top", "cactus_bottom"})
                      .setHardness(0.4f));

    registerBlock(BlockType()
                      .setName("sandstone")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("sandstone")
                      .setHardness(0.8f));

    registerBlock(BlockType()
                      .setName("bedrock")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("bedrock")
                      .setHardness(1000.0f));

    registerBlock(BlockType()
                      .setName("coal_ore")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("coal_ore")
                      .setHardness(3.0f));

    registerBlock(BlockType()
                      .setName("iron_ore")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("iron_ore")
                      .setHardness(3.0f));

    registerBlock(BlockType()
                      .setName("gold_ore")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("gold_ore")
                      .setHardness(3.0f));

    registerBlock(BlockType()
                      .setName("diamond_ore")
                      .setSolid(true)
                      .setOpaque(true)
                      .setTransparent(false)
                      .setTextureAllFaces("diamond_ore")
                      .setHardness(3.0f));

    registerBlock(BlockType()
                      .setName("tall_grass")
                      .setSolid(false)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("tall_grass")
                      .setHardness(0.0f));

    registerBlock(BlockType()
                      .setName("vines")
                      .setSolid(false)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("vines")
                      .setHardness(0.2f));

    registerBlock(BlockType()
                      .setName("lava")
                      .setSolid(false)
                      .setOpaque(false)
                      .setTransparent(true)
                      .setTextureAllFaces("lava")
                      .setLightEmission(15)
                      .setHardness(100.0f));
}

void BlockRegistry::clear() {
    std::scoped_lock lock(mutex);

    PC_INFO("Clearing BlockRegistry...");
    blocks.clear();
    nameToId.clear();
    nextId = 1;

    blocks.emplace(0, airBlock);
    nameToId.emplace("air", 0);
}

uint16_t BlockRegistry::registerBlock(BlockType block) {
    std::scoped_lock lock(mutex);

    if (block.name.empty()) {
        PC_ERROR("Attempted to register block with empty name.");
        return 0;
    }

    if (nameToId.find(block.name) != nameToId.end()) {
        PC_ERROR("Block with name '" + block.name + "' is already registered.");
        return nameToId[block.name];
    }

    if (block.id == 0) {
        block.id = nextId++;
    } else if (blocks.find(block.id) != blocks.end()) {
        PC_ERROR("Block with ID " + std::to_string(block.id) + " is already registered.");
        return block.id;
    }

    blocks.emplace(block.id, block);
    nameToId.emplace(block.name, block.id);

    PC_INFO("Registered block '" + block.name + "' (ID: " + std::to_string(block.id) + ")");
    return block.id;
}

const BlockType& BlockRegistry::getBlock(uint16_t id) const {
    std::scoped_lock lock(mutex);

    auto it = blocks.find(id);
    if (it != blocks.end()) {
        return it->second;
    }

    PC_WARN("Requested block ID " + std::to_string(id) + " not found. Returning AIR.");
    return airBlock;
}

const BlockType* BlockRegistry::getBlockByName(const std::string& name) const {
    std::scoped_lock lock(mutex);

    auto it = nameToId.find(name);
    if (it == nameToId.end()) {
        return nullptr;
    }

    auto blockIt = blocks.find(it->second);
    if (blockIt == blocks.end()) {
        return nullptr;
    }

    return &blockIt->second;
}

uint16_t BlockRegistry::getBlockID(const std::string& name) const {
    std::scoped_lock lock(mutex);

    auto it = nameToId.find(name);
    if (it == nameToId.end()) {
        return 0;
    }

    return it->second;
}

bool BlockRegistry::hasBlock(uint16_t id) const {
    std::scoped_lock lock(mutex);
    return blocks.find(id) != blocks.end();
}

bool BlockRegistry::hasBlock(const std::string& name) const {
    std::scoped_lock lock(mutex);
    return nameToId.find(name) != nameToId.end();
}

size_t BlockRegistry::getBlockCount() const {
    std::scoped_lock lock(mutex);
    return blocks.size();
}

} // namespace PoorCraft
