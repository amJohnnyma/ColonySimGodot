#include "terrainGenerator.h"
#include <algorithm>
#include <cmath>

TerrainGenerator::TerrainGenerator(unsigned int seed) : noise(seed) {}

Color TerrainGenerator::generate_tile_color(int world_x, int world_y) {
    // Sample biome parameters
    double temp = noise.noise(world_x * 0.003, world_y * 0.003) * 2.0 - 1.0;
    double moisture = noise.noise(world_x * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0;
    
    // Calculate elevation with features
    float combined_elevation = calculate_elevation(world_x, world_y);
   /* 
    // Detect rivers
    float river_mask = get_river_mask(world_x, world_y);
    
    // Rivers lower the elevation to create valleys
    if (river_mask > 0.0f) {
        combined_elevation = combined_elevation * (1.0f - river_mask * 0.6f);
    }
    
    // Detect sand banks
    float sand_mask = get_sand_mask(combined_elevation, river_mask);
    sand_mask = std::max(0.0f, std::min(1.0f, sand_mask));
   */ 
    // Get base terrain color with biome blending
    Color final_color = blend_biomes(world_x, world_y, combined_elevation);
    
   /* 
    // Apply sand banks
    if (sand_mask > 0.0f) {
        final_color = Color(
            final_color.r * (1.0f - sand_mask) + SAND_COLOR[0] * sand_mask,
            final_color.g * (1.0f - sand_mask) + SAND_COLOR[1] * sand_mask,
            final_color.b * (1.0f - sand_mask) + SAND_COLOR[2] * sand_mask,
            1.0f
        );
    }
    // Apply rivers (override terrain)
    if (river_mask > 0.5f) {
        float river_blend = (river_mask - 0.5f) * 2.0f;
        river_blend = std::min(1.0f, river_blend);
        
        final_color = Color(
            final_color.r * (1.0f - river_blend) + RIVER_COLOR[0] * river_blend,
            final_color.g * (1.0f - river_blend) + RIVER_COLOR[1] * river_blend,
            final_color.b * (1.0f - river_blend) + RIVER_COLOR[2] * river_blend,
            1.0f
        );
    }
    */
    // Add subtle variation
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

BiomeType TerrainGenerator::get_biome_type(int world_x, int world_y)
{

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
    const BiomeData& bd = BIOME_TABLE[(int)biome];
    
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
        noise.noise(world_x * 0.003 + 1000, (world_y - 10) * 0.003 + 1000) * 2.0 - 1.0
    );
    BiomeType east_biome = get_biome_from_params(
        noise.noise((world_x + 10) * 0.003, world_y * 0.003) * 2.0 - 1.0,
        noise.noise((world_x + 10) * 0.003 + 1000, world_y * 0.003 + 1000) * 2.0 - 1.0
    );
    
    // temporary ?
        return get_biome_color(center_biome, elevation);
    if (center_biome == north_biome && center_biome == east_biome) {
        return get_biome_color(center_biome, elevation);
    } else {
        float center_weight = 0.9f;  // Adjust for more/less blending
        float north_weight = (1.0f - center_weight) * 0.5f;
        float east_weight = (1.0f - center_weight) * 0.5f;
        
        Color center_color = get_biome_color(center_biome, elevation);
        Color north_color = get_biome_color(north_biome, elevation);
        Color east_color = get_biome_color(east_biome, elevation);
        
        return Color(
            center_color.r * center_weight + north_color.r * north_weight + east_color.r * east_weight,
            center_color.g * center_weight + north_color.g * north_weight + east_color.g * east_weight,
            center_color.b * center_weight + north_color.b * north_weight + east_color.b * east_weight,
            1.0f
        );
    }
}

float TerrainGenerator::smoothstep(float x) {
    x = std::max(0.0f, std::min(1.0f, (x + 1.0f) * 0.5f));
    return x * x * (3.0f - 2.0f * x);
}

Color TerrainGenerator::lerp_color(const float* color1, const float* color2, float t) {
    return Color(
        color1[0] * (1.0f - t) + color2[0] * t,
        color1[1] * (1.0f - t) + color2[1] * t,
        color1[2] * (1.0f - t) + color2[2] * t,
        1.0f
    );
}
