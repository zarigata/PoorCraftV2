#include "poorcraft/network/PacketSerializer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
constexpr std::uint8_t QUAT_LARGEST_MASK = 0x3;
constexpr std::uint8_t QUAT_SIGN_MASK = 0x4;
constexpr float QUAT_SCALE = 32767.0f;

inline bool isLittleEndian() {
    const std::uint16_t value = 0x1;
    return reinterpret_cast<const std::uint8_t*>(&value)[0] == 0x1;
}

template <typename T>
T swapEndian(T value) {
    T result;
    auto valuePtr = reinterpret_cast<std::uint8_t*>(&value);
    auto resultPtr = reinterpret_cast<std::uint8_t*>(&result);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        resultPtr[i] = valuePtr[sizeof(T) - 1 - i];
    }
    return result;
}

template <typename T>
void writeScalar(std::vector<std::uint8_t>& buffer, T value) {
    if (!isLittleEndian()) {
        value = swapEndian(value);
    }

    const std::size_t offset = buffer.size();
    buffer.resize(offset + sizeof(T));
    std::memcpy(buffer.data() + offset, &value, sizeof(T));
}

template <typename T>
T readScalar(const std::uint8_t* data, std::size_t offset) {
    T value;
    std::memcpy(&value, data + offset, sizeof(T));
    if (!isLittleEndian()) {
        value = swapEndian(value);
    }
    return value;
}
}

PacketWriter::PacketWriter(std::size_t initialCapacity) {
    m_Buffer.reserve(initialCapacity);
}

void PacketWriter::writeUInt8(std::uint8_t value) {
    m_Buffer.push_back(value);
}

void PacketWriter::writeUInt16(std::uint16_t value) {
    writeScalar(m_Buffer, value);
}

void PacketWriter::writeUInt32(std::uint32_t value) {
    writeScalar(m_Buffer, value);
}

void PacketWriter::writeUInt64(std::uint64_t value) {
    writeScalar(m_Buffer, value);
}

void PacketWriter::writeInt8(std::int8_t value) {
    writeUInt8(static_cast<std::uint8_t>(value));
}

void PacketWriter::writeInt16(std::int16_t value) {
    writeUInt16(static_cast<std::uint16_t>(value));
}

void PacketWriter::writeInt32(std::int32_t value) {
    writeUInt32(static_cast<std::uint32_t>(value));
}

void PacketWriter::writeFloat(float value) {
    writeScalar(m_Buffer, value);
}

void PacketWriter::writeDouble(double value) {
    writeScalar(m_Buffer, value);
}

void PacketWriter::writeString(const std::string& value) {
    const std::uint16_t length = static_cast<std::uint16_t>(std::min<std::size_t>(value.size(), std::numeric_limits<std::uint16_t>::max()));
    writeUInt16(length);
    const std::size_t offset = m_Buffer.size();
    m_Buffer.resize(offset + length);
    std::memcpy(m_Buffer.data() + offset, value.data(), length);
}

void PacketWriter::writeVec3(const glm::vec3& value) {
    writeFloat(value.x);
    writeFloat(value.y);
    writeFloat(value.z);
}

void PacketWriter::writeQuat(const glm::quat& value) {
    writeFloat(value.x);
    writeFloat(value.y);
    writeFloat(value.z);
    writeFloat(value.w);
}

void PacketWriter::writeVec3Quantized(const glm::vec3& value, float precision) {
    writeInt16(quantizePositionComponent(value.x, precision));
    writeInt16(quantizePositionComponent(value.y, precision));
    writeInt16(quantizePositionComponent(value.z, precision));
}

void PacketWriter::writeQuatCompressed(const glm::quat& value) {
    glm::quat normalized = glm::normalize(value);
    float components[4] = { normalized.x, normalized.y, normalized.z, normalized.w };

    std::uint8_t largestIdx = 0;
    float largestValue = std::abs(components[0]);
    for (std::uint8_t i = 1; i < 4; ++i) {
        const float absValue = std::abs(components[i]);
        if (absValue > largestValue) {
            largestIdx = i;
            largestValue = absValue;
        }
    }

    const bool isNegative = components[largestIdx] < 0.0f;

    std::uint8_t header = largestIdx & QUAT_LARGEST_MASK;
    if (isNegative) {
        header |= QUAT_SIGN_MASK;
    }
    writeUInt8(header);

    for (std::uint8_t i = 0; i < 4; ++i) {
        if (i == largestIdx) {
            continue;
        }
        float valueComponent = components[i];
        if (isNegative) {
            valueComponent = -valueComponent;
        }
        const std::int16_t quantized = static_cast<std::int16_t>(std::round(std::clamp(valueComponent, -1.0f, 1.0f) * QUAT_SCALE));
        writeInt16(quantized);
    }
}

const std::uint8_t* PacketWriter::getData() const {
    return m_Buffer.data();
}

std::size_t PacketWriter::getSize() const {
    return m_Buffer.size();
}

void PacketWriter::reset() {
    m_Buffer.clear();
}

PacketReader::PacketReader(const std::uint8_t* data, std::size_t size)
    : m_Data(data), m_Size(size), m_Position(0) {}

std::uint8_t PacketReader::readUInt8() {
    if (!ensureAvailable(sizeof(std::uint8_t))) {
        PC_ERROR("PacketReader::readUInt8 overflow");
        return 0;
    }
    return m_Data[m_Position++];
}

std::uint16_t PacketReader::readUInt16() {
    if (!ensureAvailable(sizeof(std::uint16_t))) {
        PC_ERROR("PacketReader::readUInt16 overflow");
        return 0;
    }
    std::uint16_t value = readScalar<std::uint16_t>(m_Data, m_Position);
    m_Position += sizeof(std::uint16_t);
    return value;
}

std::uint32_t PacketReader::readUInt32() {
    if (!ensureAvailable(sizeof(std::uint32_t))) {
        PC_ERROR("PacketReader::readUInt32 overflow");
        return 0;
    }
    std::uint32_t value = readScalar<std::uint32_t>(m_Data, m_Position);
    m_Position += sizeof(std::uint32_t);
    return value;
}

std::uint64_t PacketReader::readUInt64() {
    if (!ensureAvailable(sizeof(std::uint64_t))) {
        PC_ERROR("PacketReader::readUInt64 overflow");
        return 0;
    }
    std::uint64_t value = readScalar<std::uint64_t>(m_Data, m_Position);
    m_Position += sizeof(std::uint64_t);
    return value;
}

std::int8_t PacketReader::readInt8() {
    return static_cast<std::int8_t>(readUInt8());
}

std::int16_t PacketReader::readInt16() {
    return static_cast<std::int16_t>(readUInt16());
}

std::int32_t PacketReader::readInt32() {
    return static_cast<std::int32_t>(readUInt32());
}

float PacketReader::readFloat() {
    if (!ensureAvailable(sizeof(float))) {
        PC_ERROR("PacketReader::readFloat overflow");
        return 0.0f;
    }
    float value = readScalar<float>(m_Data, m_Position);
    m_Position += sizeof(float);
    return value;
}

double PacketReader::readDouble() {
    if (!ensureAvailable(sizeof(double))) {
        PC_ERROR("PacketReader::readDouble overflow");
        return 0.0;
    }
    double value = readScalar<double>(m_Data, m_Position);
    m_Position += sizeof(double);
    return value;
}

std::string PacketReader::readString() {
    const std::uint16_t length = readUInt16();
    if (!ensureAvailable(length)) {
        PC_ERROR("PacketReader::readString overflow");
        return {};
    }
    std::string result(reinterpret_cast<const char*>(m_Data + m_Position), length);
    m_Position += length;
    return result;
}

glm::vec3 PacketReader::readVec3() {
    return { readFloat(), readFloat(), readFloat() };
}

glm::quat PacketReader::readQuat() {
    return { readFloat(), readFloat(), readFloat(), readFloat() };
}

glm::vec3 PacketReader::readVec3Quantized(float precision) {
    const float x = dequantizePositionComponent(readInt16(), precision);
    const float y = dequantizePositionComponent(readInt16(), precision);
    const float z = dequantizePositionComponent(readInt16(), precision);
    return { x, y, z };
}

glm::quat PacketReader::readQuatCompressed() {
    if (!ensureAvailable(sizeof(std::uint8_t) + sizeof(std::int16_t) * 3)) {
        PC_ERROR("PacketReader::readQuatCompressed overflow");
        return glm::quat();
    }

    const std::uint8_t header = readUInt8();
    const std::uint8_t largestIdx = header & QUAT_LARGEST_MASK;
    const bool isNegative = (header & QUAT_SIGN_MASK) != 0;

    float components[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float sumSquares = 0.0f;

    for (std::uint8_t i = 0, written = 0; i < 4; ++i) {
        if (i == largestIdx) {
            continue;
        }
        const std::int16_t quantized = readInt16();
        float value = static_cast<float>(quantized) / QUAT_SCALE;
        components[i] = value;
        sumSquares += value * value;
        ++written;
    }

    components[largestIdx] = std::sqrt(std::max(0.0f, 1.0f - sumSquares));
    if (isNegative) {
        components[largestIdx] = -components[largestIdx];
    }

    return glm::quat(components[3], components[0], components[1], components[2]);
}

bool PacketReader::hasMoreData() const {
    return m_Position < m_Size;
}

std::size_t PacketReader::getPosition() const {
    return m_Position;
}

void PacketReader::skip(std::size_t bytes) {
    if (!ensureAvailable(bytes)) {
        PC_ERROR("PacketReader::skip overflow");
        return;
    }
    m_Position += bytes;
}

bool PacketReader::ensureAvailable(std::size_t bytes) {
    return m_Position + bytes <= m_Size;
}

std::int16_t quantizePositionComponent(float value, float precision) {
    const float scaled = value / precision;
    const float clamped = std::clamp(scaled, static_cast<float>(std::numeric_limits<std::int16_t>::min()),
                                     static_cast<float>(std::numeric_limits<std::int16_t>::max()));
    return static_cast<std::int16_t>(std::round(clamped));
}

float dequantizePositionComponent(std::int16_t value, float precision) {
    return static_cast<float>(value) * precision;
}

} // namespace PoorCraft
