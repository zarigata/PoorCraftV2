#pragma once

#include "poorcraft/world/BiomeType.h"

#include <cstdint>
#include <utility>
#include <vector>

#include <FastNoiseLite.h>

namespace PoorCraft {

class BiomeMap {
public:
    explicit BiomeMap(int64_t seed);
    BiomeMap(const BiomeMap&) = delete;
    BiomeMap& operator=(const BiomeMap&) = delete;
    BiomeMap(BiomeMap&&) noexcept = default;
    BiomeMap& operator=(BiomeMap&&) noexcept = default;
    ~BiomeMap() = default;

    [[nodiscard]] BiomeType getBiomeAt(int32_t worldX, int32_t worldZ) const;
    [[nodiscard]] float getTemperature(int32_t worldX, int32_t worldZ) const;
    [[nodiscard]] float getHumidity(int32_t worldX, int32_t worldZ) const;
    [[nodiscard]] std::vector<std::pair<BiomeType, float>> getBlendedBiomes(int32_t worldX, int32_t worldZ) const;

    void setBiomeScale(float scale);

private:
    [[nodiscard]] BiomeType selectBiome(float temperature, float humidity) const;

    int64_t seed;
    FastNoiseLite temperatureNoise;
    FastNoiseLite humidityNoise;
    FastNoiseLite biomeNoise;
    float biomeScale;
};

} // namespace PoorCraft
