#include "poorcraft/world/BiomeMap.h"

#include "poorcraft/world/BiomeType.h"

#include <algorithm>
#include <cmath>

namespace PoorCraft {

namespace {
constexpr float INV_DISTANCE_CONSTANT = 1.0f;
constexpr int SAMPLE_RADIUS = 1;
constexpr int SAMPLE_STEP = 16;

[[nodiscard]] float normalizeNoise(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

[[nodiscard]] float computeWeight(float dx, float dz) {
    const float distance = std::sqrt(dx * dx + dz * dz);
    return 1.0f / (INV_DISTANCE_CONSTANT + distance);
}

[[nodiscard]] float latitudeBias(int32_t z) {
    return static_cast<float>(z) * -0.00001f;
}

} // namespace

BiomeMap::BiomeMap(int64_t worldSeed)
    : seed(worldSeed), temperatureNoise(), humidityNoise(), biomeNoise(), biomeScale(1.0f) {
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

    biomeNoise.SetSeed(static_cast<int32_t>(seed + 2000));
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    biomeNoise.SetFrequency(0.001f);
    biomeNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    biomeNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
}

void BiomeMap::setBiomeScale(float scale) {
    if (scale <= 0.0f) {
        return;
    }
    biomeScale = scale;
    const float temperatureFrequency = 0.0008f * biomeScale;
    const float humidityFrequency = 0.0008f * biomeScale;
    const float biomeFrequency = 0.001f * biomeScale;
    temperatureNoise.SetFrequency(temperatureFrequency);
    humidityNoise.SetFrequency(humidityFrequency);
    biomeNoise.SetFrequency(biomeFrequency);
}

BiomeType BiomeMap::selectBiome(float temperature, float humidity) const {
    if (temperature < -0.5f) {
        return BiomeType::SNOW;
    }

    if (temperature > 0.6f && humidity < -0.3f) {
        return BiomeType::DESERT;
    }

    if (temperature > 0.6f && humidity > 0.5f) {
        return BiomeType::JUNGLE;
    }

    if (std::abs(temperature) < 0.3f && std::abs(humidity) < 0.3f) {
        return BiomeType::MOUNTAINS;
    }

    return BiomeType::PLAINS;
}

BiomeType BiomeMap::getBiomeAt(int32_t worldX, int32_t worldZ) const {
    const float baseTemperature = temperatureNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    const float baseHumidity = humidityNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));

    const float temperature = normalizeNoise(baseTemperature + latitudeBias(worldZ));
    const float humidity = normalizeNoise(baseHumidity);

    return selectBiome(temperature, humidity);
}

float BiomeMap::getTemperature(int32_t worldX, int32_t worldZ) const {
    const float base = temperatureNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    const float biased = base + latitudeBias(worldZ);
    return normalizeNoise(biased);
}

float BiomeMap::getHumidity(int32_t worldX, int32_t worldZ) const {
    const float base = humidityNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    return normalizeNoise(base);
}

std::vector<std::pair<BiomeType, float>> BiomeMap::getBlendedBiomes(int32_t worldX, int32_t worldZ) const {
    std::vector<std::pair<BiomeType, float>> biomes;
    biomes.reserve(9);

    float totalWeight = 0.0f;

    for (int dz = -SAMPLE_RADIUS; dz <= SAMPLE_RADIUS; ++dz) {
        for (int dx = -SAMPLE_RADIUS; dx <= SAMPLE_RADIUS; ++dx) {
            const int32_t sampleX = worldX + dx * SAMPLE_STEP;
            const int32_t sampleZ = worldZ + dz * SAMPLE_STEP;


            const float baseTemperature = temperatureNoise.GetNoise(static_cast<float>(sampleX), static_cast<float>(sampleZ));
            const float baseHumidity = humidityNoise.GetNoise(static_cast<float>(sampleX), static_cast<float>(sampleZ));

            const float temperature = normalizeNoise(baseTemperature + latitudeBias(sampleZ));
            const float humidity = normalizeNoise(baseHumidity);

            const BiomeType biome = selectBiome(temperature, humidity);
            const float weight = computeWeight(static_cast<float>(dx), static_cast<float>(dz));

            totalWeight += weight;
            biomes.emplace_back(biome, weight);
        }
    }

    if (totalWeight > 0.0f) {
        for (auto& biomeWeight : biomes) {
            biomeWeight.second /= totalWeight;
        }
    }

    std::sort(biomes.begin(), biomes.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    if (biomes.size() > 3) {
        biomes.resize(3);
    }

    return biomes;
}

} // namespace PoorCraft
