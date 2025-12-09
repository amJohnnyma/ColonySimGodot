#pragma once
#include "entity.h"
#include "perlinNoise.h"
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <vector>
#include <memory>
#include <cstdlib>

using namespace godot;

struct Chunk {
    Vector2i coord;
    int width;
    int height;
    std::vector<int> tiles;
    std::vector<Color> tileColors;
    std::vector<std::shared_ptr<Entity>> entities;

    Chunk(int w, int h, Vector2i c) : coord(c), width(w), height(h) {
        tiles.resize(width * height, 0);
        tileColors.resize(width * height, Color(1,1,1,1)); // initialize colors
    }

    void generate(int wx, int wy) {
        PerlinNoise noise(12345);
        BiomeType b = noise.get_biome(wx, wy);
        const float* col = BIOME_TABLE[(int)b].color;

        entities.clear();           // Start fresh
        entities.reserve(width * height);  // Pre-allocate for speed
        
        bool flag = false;
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                tiles[y * width + x] = (x + y + coord.x + coord.y) % 3;
                double n = noise.noise(
                    double(x + coord.x * width) * 0.05,
                    double(y + coord.y * height) * 0.05
                );
                float v = float((n + 1.0) * 0.5);
                tileColors[y * width + x] = Color(v * col[0], v * col[1], v * col[2], col[3]);
                if(flag) {
                    Vector2 world_pos(
                        coord.x * width + x + 0.5f,   // +0.5 to center on tile
                        coord.y * height + y + 0.5f
                    );

                    auto e = std::make_shared<Entity>(world_pos, true);
                    entities.push_back(e);
                    flag = false;

                }
                else{
                    flag = true;
                }
            }
    }

    UtilityFunctions::print("Generated chunk ", coord, " with ", entities.size(), " entities");
}
    
    void simulate(float delta, bool full_simulation=true){
        for(auto &e:entities)
            if(e && e->active) e->simulate(delta);
    }

    int get_tile(int local_x,int local_y) const{
        if(local_x<0 || local_y<0 || local_x>=width || local_y>=height) return 0;
        return tiles[local_y*width+local_x];
    }

    void set_tile(int local_x,int local_y,int value){
        if(local_x<0 || local_y<0 || local_x>=width || local_y>=height) return;
        tiles[local_y*width+local_x] = value;
    }

    std::vector<Color> get_tile_colors() const{
        return tileColors;
    }
};
