#include "poorcraft/world/BiomeMap.h"

#include "poorcraft/world/BiomeType.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace PoorCraft {

namespace {
constexpr float LATITUDE_BIAS_SCALE = -0.00001f;
constexpr int MAX_CELL_SAMPLES = 4;

[[nodiscard]] float normalizeNoise(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

[[nodiscard]] float latitudeBias(int32_t z) {
    return static_cast<float>(z) * LATITUDE_BIAS_SCALE;
}

[[nodiscard]] std::array<std::pair<float, float>, 8> sampleDirections() {
    constexpr float INV_SQRT_2 = 0.70710678118f;
    return {{{1.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f},
             {INV_SQRT_2, INV_SQRT_2}, {-INV_SQRT_2, -INV_SQRT_2},
             {INV_SQRT_2, -INV_SQRT_2}, {-INV_SQRT_2, INV_SQRT_2}}};
}

} // namespace

BiomeMap::BiomeMap(int64_t worldSeed)
    : seed(worldSeed), temperatureNoise(), humidityNoise(), biomeCellNoise(), biomeDistanceNoise(), elevationNoise(), biomeScale(1.0f),
      biomeFrequency(0.001f) {
    temperatureNoise.SetSeed(static_cast<int32_t>(seed));
    temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    temperatureNoise.SetFrequency(0.0008f);
    temperatureNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    temperatureNoise.SetFractalOctaves(4);

    humidityNoise.SetSeed(static_cast<int32_t>(seed + 1000));
    humidityNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    humidityNoise.SetFrequency(0.0008f);
    humidityNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    humidityNoise.SetFractalOctaves(4);

    biomeCellNoise.SetSeed(static_cast<int32_t>(seed + 2000));
    biomeCellNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    biomeCellNoise.SetFrequency(biomeFrequency);
    biomeCellNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    biomeCellNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    biomeCellNoise.SetCellularJitter(1.0f);

    biomeDistanceNoise.SetSeed(static_cast<int32_t>(seed + 2000));
    biomeDistanceNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    biomeDistanceNoise.SetFrequency(biomeFrequency);
    biomeDistanceNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    biomeDistanceNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    biomeDistanceNoise.SetCellularJitter(1.0f);

    elevationNoise.SetSeed(static_cast<int32_t>(seed + 3000));
    elevationNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    elevationNoise.SetFrequency(0.0012f);
    elevationNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    elevationNoise.SetFractalOctaves(5);
}

void BiomeMap::setBiomeScale(float scale) {
    if (scale <= 0.0f) {
        return;
    }
    biomeScale = scale;
    const float temperatureFrequency = 0.0008f * biomeScale;
    const float humidityFrequency = 0.0008f * biomeScale;
    biomeFrequency = 0.001f * biomeScale;
    temperatureNoise.SetFrequency(temperatureFrequency);
    humidityNoise.SetFrequency(humidityFrequency);
    biomeCellNoise.SetFrequency(biomeFrequency);
    biomeDistanceNoise.SetFrequency(biomeFrequency);
    elevationNoise.SetFrequency(0.0012f * biomeScale);
}

BiomeType BiomeMap::selectBiome(float temperature, float humidity, float elevation) const {
    const float temp01 = std::clamp((temperature + 1.0f) * 0.5f, 0.0f, 1.0f);
    const float humidity01 = std::clamp((humidity + 1.0f) * 0.5f, 0.0f, 1.0f);

    const auto& orderedBiomes = {
        BiomeType::MOUNTAINS,
        BiomeType::SNOW,
        BiomeType::DESERT,
        BiomeType::JUNGLE,
        BiomeType::PLAINS
    };

    for (const auto biome : orderedBiomes) {
        const auto& def = getBiomeDefinition(biome);
        if (temp01 < def.temperatureRange.min || temp01 > def.temperatureRange.max ||
            humidity01 < def.humidityRange.min || humidity01 > def.humidityRange.max) {
            continue;
        }

        if (biome == BiomeType::MOUNTAINS) {
            if (elevation >= 0.65f) {
                return biome;
            }
            continue;
        }

        return biome;
    }

    if (elevation >= 0.7f) {
        return BiomeType::MOUNTAINS;
    }

    if (temp01 <= 0.25f) {
        return BiomeType::SNOW;
    }

    if (temp01 >= 0.75f && humidity01 <= 0.35f) {
        return BiomeType::DESERT;
    }

    if (temp01 >= 0.75f && humidity01 >= 0.65f) {
        return BiomeType::JUNGLE;
    }

    return BiomeType::PLAINS;
}

BiomeType BiomeMap::getBiomeAt(int32_t worldX, int32_t worldZ) const {
    return computeBiomeForPosition(static_cast<float>(worldX), static_cast<float>(worldZ));
}

float BiomeMap::getTemperature(int32_t worldX, int32_t worldZ) const {
    return sampleTemperature(static_cast<float>(worldX), static_cast<float>(worldZ));
}

float BiomeMap::getHumidity(int32_t worldX, int32_t worldZ) const {
    return sampleHumidity(static_cast<float>(worldX), static_cast<float>(worldZ));
}

float BiomeMap::getElevation(int32_t worldX, int32_t worldZ) const {
    return sampleElevation(static_cast<float>(worldX), static_cast<float>(worldZ));
}

std::vector<std::pair<BiomeType, float>> BiomeMap::getBlendedBiomes(int32_t worldX, int32_t worldZ) const {
    const float fx = static_cast<float>(worldX);
    const float fz = static_cast<float>(worldZ);

    const float baseCellValue = sampleCellValue(fx, fz);
    const int32_t baseCellId = cellValueToId(baseCellValue);
    const BiomeType baseBiome = computeBiomeForPosition(fx, fz);

    std::vector<std::pair<BiomeType, float>> weightedBiomes;
    weightedBiomes.reserve(MAX_CELL_SAMPLES);

    auto accumulateWeight = [&weightedBiomes](BiomeType biome, float weight) {
        if (weight <= 0.0f) {
            return;
        }
        for (auto& entry : weightedBiomes) {
            if (entry.first == biome) {
                entry.second += weight;
                return;
            }
        }
        weightedBiomes.emplace_back(biome, weight);
    };

    const float cellSize = 1.0f / std::max(biomeFrequency, 0.0001f);
    const float stepDistance = cellSize * 0.5f;
    const float maxSearchDistance = cellSize * 4.0f;

    float minBorderDistance = std::numeric_limits<float>::max();

    const auto directions = sampleDirections();
    for (const auto& dir : directions) {
        if (weightedBiomes.size() >= static_cast<size_t>(MAX_CELL_SAMPLES)) {
            break;
        }

        float currentDistance = stepDistance;
        float lastInsideDistance = 0.0f;
        bool foundNeighbor = false;

        while (currentDistance <= maxSearchDistance) {
            const float sampleX = fx + dir.first * currentDistance;
            const float sampleZ = fz + dir.second * currentDistance;
            const float cellValue = sampleCellValue(sampleX, sampleZ);
            if (cellValueToId(cellValue) == baseCellId) {
                lastInsideDistance = currentDistance;
                currentDistance += stepDistance;
                continue;
            }

            float low = lastInsideDistance;
            float high = currentDistance;
            for (int i = 0; i < 5; ++i) {
                const float mid = 0.5f * (low + high);
                const float midValue = sampleCellValue(fx + dir.first * mid, fz + dir.second * mid);
                if (cellValueToId(midValue) == baseCellId) {
                    low = mid;
                } else {
                    high = mid;
                }
            }

            const float borderDistance = high;
            minBorderDistance = std::min(minBorderDistance, borderDistance);

            const float neighborSampleDistance = borderDistance + stepDistance * 0.25f;
            const float neighborX = fx + dir.first * neighborSampleDistance;
            const float neighborZ = fz + dir.second * neighborSampleDistance;
            const BiomeType neighborBiome = computeBiomeForPosition(neighborX, neighborZ);

            const float neighborWeight = 1.0f / std::max(borderDistance, 0.001f);
            accumulateWeight(neighborBiome, neighborWeight);
            foundNeighbor = true;
            break;
        }

        if (!foundNeighbor) {
            const float fallbackDistance = sampleCellDistance(fx, fz);
            minBorderDistance = std::min(minBorderDistance, fallbackDistance);
        }
    }

    if (!std::isfinite(minBorderDistance) || minBorderDistance == std::numeric_limits<float>::max()) {
        minBorderDistance = sampleCellDistance(fx, fz);
    }

    const float baseWeight = 1.0f / std::max(minBorderDistance, 0.001f);
    accumulateWeight(baseBiome, baseWeight);

    float totalWeight = 0.0f;
    for (const auto& entry : weightedBiomes) {
        totalWeight += entry.second;
    }

    if (totalWeight > 0.0f) {
        for (auto& entry : weightedBiomes) {
            entry.second /= totalWeight;
        }
    }

    std::sort(weightedBiomes.begin(), weightedBiomes.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    if (weightedBiomes.size() > 3) {
        weightedBiomes.resize(3);
    }

    return weightedBiomes;
}

float BiomeMap::sampleTemperature(float worldX, float worldZ) const {
    const float base = temperatureNoise.GetNoise(worldX, worldZ);
    return normalizeNoise(base + latitudeBias(static_cast<int32_t>(worldZ)));
}

float BiomeMap::sampleHumidity(float worldX, float worldZ) const {
    const float base = humidityNoise.GetNoise(worldX, worldZ);
    return normalizeNoise(base);
}

float BiomeMap::sampleCellValue(float worldX, float worldZ) const {
    return biomeCellNoise.GetNoise(worldX, worldZ);
}

float BiomeMap::sampleCellDistance(float worldX, float worldZ) const {
    return std::max(0.0f, biomeDistanceNoise.GetNoise(worldX, worldZ) + 1.0f);
}

float BiomeMap::sampleElevation(float worldX, float worldZ) const {
    const float base = elevationNoise.GetNoise(worldX, worldZ);
    return normalizeNoise(base * 0.5f + 0.5f);
}

BiomeType BiomeMap::computeBiomeForPosition(float worldX, float worldZ) const {
    const float temperature = sampleTemperature(worldX, worldZ);
    const float humidity = sampleHumidity(worldX, worldZ);
    const float elevation = sampleElevation(worldX, worldZ);
    return selectBiome(temperature, humidity, elevation);
}

int32_t BiomeMap::cellValueToId(float cellValue) const {
    return static_cast<int32_t>(std::floor((cellValue + 1.0f) * 0.5f * 1024.0f));
}

} // namespace PoorCraft
