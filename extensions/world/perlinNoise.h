#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

enum class BiomeType {
    FOREST,
    MOUNTAIN,
    OCEAN,
    TUNDRA,
    COUNT
};

struct BiomeData {
    float base_color[4];
    float low_color[4];
    float mid_color[4];
    float high_color[4];
    float tree_density;
};

static const BiomeData BIOME_TABLE[(int)BiomeType::COUNT] = {
    {{0.3f, 0.6f, 0.3f, 1.0f}, {0.2f, 0.5f, 0.2f, 1.0f}, {0.4f, 0.7f, 0.3f, 1.0f}, {0.3f, 0.55f, 0.25f, 1.0f}, 0.01f},
    {{0.6f, 0.55f, 0.4f, 1.0f}, {0.5f, 0.45f, 0.3f, 1.0f}, {0.6f, 0.6f, 0.55f, 1.0f}, {0.9f, 0.9f, 0.95f, 1.0f}, 0.001f},
    {{0.1f, 0.3f, 0.6f, 1.0f}, {0.05f, 0.2f, 0.5f, 1.0f}, {0.2f, 0.4f, 0.7f, 1.0f}, {0.3f, 0.7f, 0.8f, 1.0f}, 0.0f},
    {{0.8f, 0.85f, 0.95f, 1.0f}, {0.7f, 0.75f, 0.85f, 1.0f}, {0.85f, 0.9f, 0.98f, 1.0f}, {0.95f, 0.95f, 1.0f, 1.0f}, 0.001f}};

const float RIVER_COLOR[4] = {0.2f, 0.5f, 0.8f, 1.0f};
const float SAND_COLOR[4] = {0.9f, 0.85f, 0.6f, 1.0f};

class PerlinNoise {
private:
    std::vector<int> p;

    static float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    static float grad(int hash, float x, float y) {
        switch (hash & 3) {
        case 0:
            return x + y;
        case 1:
            return -x + y;
        case 2:
            return x - y;
        case 3:
            return -x - y;
        default:
            return 0;
        }
    }

    float elevation(float nx, float ny, int layers);
    float amplitude(float nx, float ny, int fac, double multFac);

public:
    PerlinNoise(unsigned int seed = std::default_random_engine::default_seed);
    ~PerlinNoise();
    float noise(float x, float y);
    float val(float x, float y);
    BiomeType get_biome(int wx, int wy);
};

#endif
