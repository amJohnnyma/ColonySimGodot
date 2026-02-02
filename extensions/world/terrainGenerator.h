#pragma once

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <tuple>
#include <vector>

#include "perlinNoise.h"

using namespace godot;

class TerrainGenerator {
private:
    PerlinNoise noise;

    float calculate_elevation(int world_x, int world_y);
    Color blend_biomes(int world_x, int world_y, float elevation);
    float smoothstep(float x);
    Color lerp_color(const float *color1, const float *color2, float t);

public:
    TerrainGenerator(unsigned int seed);

    Color generate_tile_color(int world_x, int world_y);

    float get_river_mask(int world_x, int world_y);
    float get_mountain_ridge(int world_x, int world_y);
    float get_sand_mask(float elevation, float river_mask);

    BiomeType get_biome_from_params(double temp, double moisture);
    BiomeType get_biome_type(int world_x, int world_y);
    Color get_biome_color(BiomeType biome, float elevation);

    std::vector<std::tuple<String, Vector2i, int, int>> get_chunk_entities(int chunk_size, const Vector2i &coord);
};
