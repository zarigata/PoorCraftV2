#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace PoorCraft {

class PacketWriter {
public:
    explicit PacketWriter(std::size_t initialCapacity = 1024);

    void writeUInt8(std::uint8_t value);
    void writeUInt16(std::uint16_t value);
    void writeUInt32(std::uint32_t value);
    void writeUInt64(std::uint64_t value);

    void writeInt8(std::int8_t value);
    void writeInt16(std::int16_t value);
    void writeInt32(std::int32_t value);

    void writeFloat(float value);
    void writeDouble(double value);

    void writeString(const std::string& value);
    void writeVec3(const glm::vec3& value);
    void writeQuat(const glm::quat& value);

    void writeVec3Quantized(const glm::vec3& value, float precision = 0.01f);
    void writeQuatCompressed(const glm::quat& value);

    const std::uint8_t* getData() const;
    std::size_t getSize() const;

    void reset();

private:
    void ensureCapacity(std::size_t additionalBytes);

    std::vector<std::uint8_t> m_Buffer;
};

class PacketReader {
public:
    PacketReader(const std::uint8_t* data, std::size_t size);

    std::uint8_t readUInt8();
    std::uint16_t readUInt16();
    std::uint32_t readUInt32();
    std::uint64_t readUInt64();

    std::int8_t readInt8();
    std::int16_t readInt16();
    std::int32_t readInt32();

    float readFloat();
    double readDouble();

    std::string readString();
    glm::vec3 readVec3();
    glm::quat readQuat();

    glm::vec3 readVec3Quantized(float precision = 0.01f);
    glm::quat readQuatCompressed();

    bool hasMoreData() const;
    std::size_t getPosition() const;
    void skip(std::size_t bytes);

private:
    bool ensureAvailable(std::size_t bytes);

    const std::uint8_t* m_Data;
    std::size_t m_Size;
    std::size_t m_Position;
};

std::int16_t quantizePositionComponent(float value, float precision = 0.01f);
float dequantizePositionComponent(std::int16_t value, float precision = 0.01f);

} // namespace PoorCraft
