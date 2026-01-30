#pragma once
#include "perlinNoise.h"
#include <godot_cpp/variant/color.hpp>

using namespace godot;

class TerrainGenerator {
public:
    TerrainGenerator(unsigned int seed);
    
    // Main generation function
    Color generate_tile_color(int world_x, int world_y);
    
    // Feature detection
    float get_river_mask(int world_x, int world_y);
    float get_mountain_ridge(int world_x, int world_y);
    float get_sand_mask(float elevation, float river_mask);
    
    // Biome functions
    BiomeType get_biome_from_params(double temp, double moisture);
    BiomeType get_biome_type(int world_x, int world_y); 
    Color get_biome_color(BiomeType biome, float elevation);
    
private:
    PerlinNoise noise;
    
    // Helper functions
    float calculate_elevation(int world_x, int world_y);
    Color blend_biomes(int world_x, int world_y, float elevation);
    float smoothstep(float x);
    Color lerp_color(const float* color1, const float* color2, float t);
};
