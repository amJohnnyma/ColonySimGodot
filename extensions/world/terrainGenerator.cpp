#include "terrainGenerator.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <tuple>

#include "perlinNoise.h"

TerrainGenerator::TerrainGenerator(unsigned int seed) : noise(seed) {}

Color TerrainGenerator::generate_tile_color(int world_x, int world_y) {
    double temp = noise.noise(world_x * 0.003, world_y * 0.003) * 2.0 - 1.0;
    double moisture = noise.noise(world_x * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0;

    float combined_elevation = calculate_elevation(world_x, world_y);

    Color final_color = blend_biomes(world_x, world_y, combined_elevation);

    float variation = noise.noise(world_x * 0.3, world_y * 0.3) * 0.1f + 0.95f;
    final_color.r *= variation;
    final_color.g *= variation;
    final_color.b *= variation;

    return final_color;
}

float TerrainGenerator::calculate_elevation(int world_x, int world_y) {
    double base_elevation = noise.noise(world_x * 0.02, world_y * 0.02);
    double detail = noise.noise(world_x * 0.1, world_y * 0.1) * 0.2;
    double macro = noise.noise(world_x * 0.005, world_y * 0.005) * 0.3;

    float mountain_ridge = get_mountain_ridge(world_x, world_y);

    float combined = base_elevation + detail + macro + mountain_ridge * 0.4f;
    return std::max(0.0f, std::min(1.0f, (float)combined));
}

float TerrainGenerator::get_river_mask(int world_x, int world_y) {
    float river_noise = noise.noise(world_x * 0.008, world_y * 0.008);
    float ridge = 1.0f - std::abs(river_noise * 2.0f - 1.0f);
    ridge = std::pow(ridge, 4.0f);

    float river_width = 0.3f;
    if (ridge > river_width) {
        return (ridge - river_width) / (1.0f - river_width);
    }
    return 0.0f;
}

float TerrainGenerator::get_mountain_ridge(int world_x, int world_y) {
    float ridge_noise = noise.noise(world_x * 0.004 + 5000, world_y * 0.004 + 5000);
    float ridge = 1.0f - std::abs(ridge_noise * 2.0f - 1.0f);
    ridge = std::pow(ridge, 2.0f);
    return ridge;
}

float TerrainGenerator::get_sand_mask(float elevation, float river_mask) {
    if (river_mask > 0.1f && elevation > 0.3f && elevation < 0.5f) {
        return 1.0f - std::abs(elevation - 0.4f) * 5.0f;
    }
    return 0.0f;
}

BiomeType TerrainGenerator::get_biome_type(int world_x, int world_y) {
    double temp = noise.noise(world_x * 0.003, world_y * 0.003) * 2.0 - 1.0;
    double moisture = noise.noise(world_x * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0;

    return get_biome_from_params(temp, moisture);
}

BiomeType TerrainGenerator::get_biome_from_params(double temp, double moisture) {
    if (temp < -0.3) {
        return BiomeType::TUNDRA;
    } else if (temp > 0.3) {
        if (moisture < -0.2) {
            return BiomeType::MOUNTAIN;
        } else {
            return BiomeType::FOREST;
        }
    } else {
        if (moisture < -0.3) {
            return BiomeType::MOUNTAIN;
        } else if (moisture > 0.3) {
            return BiomeType::OCEAN;
        } else {
            return BiomeType::FOREST;
        }
    }
}

Color TerrainGenerator::get_biome_color(BiomeType biome, float elevation) {
    const BiomeData &bd = BIOME_TABLE[(int)biome];

    float r, g, b;

    if (elevation < 0.33f) {
        float t = elevation / 0.33f;
        t = t * t * (3.0f - 2.0f * t);
        r = bd.low_color[0] * (1.0f - t) + bd.mid_color[0] * t;
        g = bd.low_color[1] * (1.0f - t) + bd.mid_color[1] * t;
        b = bd.low_color[2] * (1.0f - t) + bd.mid_color[2] * t;
    } else if (elevation < 0.66f) {
        float t = (elevation - 0.33f) / 0.33f;
        t = t * t * (3.0f - 2.0f * t);
        r = bd.mid_color[0] * (1.0f - t) + bd.high_color[0] * t;
        g = bd.mid_color[1] * (1.0f - t) + bd.high_color[1] * t;
        b = bd.mid_color[2] * (1.0f - t) + bd.high_color[2] * t;
    } else {
        r = bd.high_color[0];
        g = bd.high_color[1];
        b = bd.high_color[2];
    }

    return Color(r, g, b, 1.0f);
}

Color TerrainGenerator::blend_biomes(int world_x, int world_y, float elevation) {
    double temp = noise.noise(world_x * 0.003, world_y * 0.003) * 2.0 - 1.0;
    double moisture = noise.noise(world_x * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0;

    BiomeType center_biome = get_biome_from_params(temp, moisture);
    BiomeType north_biome = get_biome_from_params(
        noise.noise(world_x * 0.003, (world_y - 10) * 0.003) * 2.0 - 1.0,
        noise.noise(world_x * 0.003 + 1000, (world_y - 10) * 0.003 + 1000) * 2.0 - 1.0);
    BiomeType east_biome = get_biome_from_params(
        noise.noise((world_x + 10) * 0.003, world_y * 0.003) * 2.0 - 1.0,
        noise.noise((world_x + 10) * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0);

    return get_biome_color(center_biome, elevation);

    if (center_biome == north_biome && center_biome == east_biome) {
        return get_biome_color(center_biome, elevation);
    } else {
        float center_weight = 0.9f;
        float north_weight = (1.0f - center_weight) * 0.5f;
        float east_weight = (1.0f - center_weight) * 0.5f;

        Color center_color = get_biome_color(center_biome, elevation);
        Color north_color = get_biome_color(north_biome, elevation);
        Color east_color = get_biome_color(east_biome, elevation);

        return Color(
            center_color.r * center_weight + north_color.r * north_weight + east_color.r * east_weight,
            center_color.g * center_weight + north_color.g * north_weight + east_color.g * east_weight,
            center_color.b * center_weight + north_color.b * north_weight + east_color.b * east_weight,
            1.0f);
    }
}

float TerrainGenerator::smoothstep(float x) {
    x = std::max(0.0f, std::min(1.0f, (x + 1.0f) * 0.5f));
    return x * x * (3.0f - 2.0f * x);
}

Color TerrainGenerator::lerp_color(const float *color1, const float *color2, float t) {
    return Color(
        color1[0] * (1.0f - t) + color2[0] * t,
        color1[1] * (1.0f - t) + color2[1] * t,
        color1[2] * (1.0f - t) + color2[2] * t,
        1.0f);
}

std::vector<std::tuple<godot::String, Vector2i, int, int>> TerrainGenerator::get_chunk_entities(int chunk_size, const Vector2i &coord) {
    std::vector<std::tuple<godot::String, Vector2i, int, int>> chunk_entities;
    chunk_entities.reserve(chunk_size * chunk_size / 20);

    // Keep track of already placed buildings in this chunk (for overlap checks)
    std::vector<std::tuple<Vector2i, int, int>> placed;  // pos, width, height
    placed.reserve(chunk_entities.capacity());

    uint32_t seed = 12345u ^ static_cast<uint32_t>(coord.x) * 0x9e3779b9u ^ static_cast<uint32_t>(coord.y) * 0x9e3779b9u * 2u;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < chunk_size; ++y) {
        for (int x = 0; x < chunk_size; ++x) {
            int world_x = x + coord.x * chunk_size;
            int world_y = y + coord.y * chunk_size;
            BiomeType biome = get_biome_type(world_x, world_y);
            const BiomeData &bd = BIOME_TABLE[static_cast<int>(biome)];

            float val = dist(gen);
            if (val > bd.tree_density) {
                continue;
            }

            Vector2i proposed_pos(world_x, world_y);
            int w = 2;
            int h = 4;

            // Check if this position + size would overlap any already placed building
            bool overlaps = false;
            for (const auto& [p_pos, p_w, p_h] : placed) {
                // Compute bounding boxes (inclusive)
                int this_left   = proposed_pos.x;
                int this_right  = proposed_pos.x + w - 1;
                int this_bottom = proposed_pos.y - h + 1;
                int this_top    = proposed_pos.y;

                int other_left   = p_pos.x;
                int other_right  = p_pos.x + p_w - 1;
                int other_bottom = p_pos.y - p_h + 1;
                int other_top    = p_pos.y;

                // Axis-Aligned Bounding Box (AABB) overlap test
                if (!(this_right  < other_left   ||   // completely left
                      this_left   > other_right  ||   // completely right
                      this_top    < other_bottom ||   // completely below
                      this_bottom > other_top))       // completely above
                {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps) {
                // Safe to place
                chunk_entities.emplace_back(
                    godot::String("building"),
                    proposed_pos,
                    w,
                    h
                );

                // Remember it for future checks in this chunk
                placed.emplace_back(proposed_pos, w, h);
            }
            // else: skip — would have overlapped
        }
    }

    return chunk_entities;
}
