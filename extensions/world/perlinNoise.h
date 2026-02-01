#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <iostream>
//https://www.redblobgames.com/maps/terrain-from-noise/
enum class BiomeType {
    FOREST,
    MOUNTAIN,
    OCEAN,
    TUNDRA,
    COUNT
};
// Enhanced biome data with elevation-based color variations
struct BiomeData {
    float base_color[4];
    float low_color[4];      // For low elevations
    float mid_color[4];      // For mid elevations  
    float high_color[4];     // For high elevations
    float tree_density;
};

static const BiomeData BIOME_TABLE[(int)BiomeType::COUNT] = {
    // FOREST: lush greens with variation
    { 
        {0.3f, 0.6f, 0.3f, 1.0f},   // base
        {0.2f, 0.5f, 0.2f, 1.0f},   // low (dark forest floor)
        {0.4f, 0.7f, 0.3f, 1.0f},   // mid (bright canopy)
        {0.3f, 0.55f, 0.25f, 1.0f}, // high (darker hills)
        0.01f 
    },
    // MOUNTAIN: browns and grays with snow caps
    { 
        {0.6f, 0.55f, 0.4f, 1.0f},  // base
        {0.5f, 0.45f, 0.3f, 1.0f},  // low (brown foothills)
        {0.6f, 0.6f, 0.55f, 1.0f},  // mid (rocky gray)
        {0.9f, 0.9f, 0.95f, 1.0f},  // high (snow caps)
        0.001f 
    },
    // OCEAN: deep blue to shallow turquoise
    { 
        {0.1f, 0.3f, 0.6f, 1.0f},   // base
        {0.05f, 0.2f, 0.5f, 1.0f},  // low (deep ocean)
        {0.2f, 0.4f, 0.7f, 1.0f},   // mid (medium depth)
        {0.3f, 0.7f, 0.8f, 1.0f},   // high (shallow/beach)
        0.0f 
    },
    // TUNDRA: icy blues and whites
    { 
        {0.8f, 0.85f, 0.95f, 1.0f}, // base
        {0.7f, 0.75f, 0.85f, 1.0f}, // low (exposed ground)
        {0.85f, 0.9f, 0.98f, 1.0f}, // mid (snow)
        {0.95f, 0.95f, 1.0f, 1.0f}, // high (pure ice)
        0.001f 
    }
};
// River colors
const float RIVER_COLOR[4] = {0.2f, 0.5f, 0.8f, 1.0f};  // Blue water
const float SAND_COLOR[4] = {0.9f, 0.85f, 0.6f, 1.0f};  // Sandy beaches

class PerlinNoise
{
    private:
        std::vector<int> p; //permutation table
        // permutation table for biomes
    public:
        PerlinNoise(unsigned int seed = std::default_random_engine::default_seed);
        ~PerlinNoise();
        float noise(float x, float y);
        float val(float x, float y);
        BiomeType get_biome(int wx, int wy);
    private:
        static float fade(float t) {
            return t * t * t * (t * (t * 6 - 15) + 10); // 6t^5 - 15t^4 + 10t^3
        }

        static float lerp(float a, float b, float t) {
            return a + t * (b - a);
        }

        // Pseudo-random gradient vector dot product
        static float grad(int hash, float x, float y) {
            switch (hash & 3) { // Pick 1 of 4 directions
                case 0: return  x + y;
                case 1: return -x + y;
                case 2: return  x - y;
                case 3: return -x - y;
                default: return 0; // unreachable
            }
        }

        float elevation(float nx, float ny, int layers);
        float amplitude(float nx, float ny, int fac, double multFac);
};

#endif
