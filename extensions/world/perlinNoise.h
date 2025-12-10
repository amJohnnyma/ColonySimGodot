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

struct BiomeData {
    float color[4];
    float temp_entity_spawn_rate;
    // texture
    // moisture
    // humidity
     
};

static const BiomeData BIOME_TABLE[(int)BiomeType::COUNT] = {
    { {0.4f, 0.8f, 0.4f, 1.0f}, 0.3f }, //forest
    { {0.9f, 0.85f, 0.4f, 1.0f}, 0.1f }, // mountain
    { {0.0f, 0.2f, 0.8f, 1.0f}, 0.0f }, // ocean
    { {0.85f, 0.8f, 1.0f, 1.0f}, 0.1f } // tundra
};

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
