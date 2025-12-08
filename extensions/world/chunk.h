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
        for(int y=0;y<height;y++)
            for(int x=0;x<width;x++){
                tiles[y*width+x] = (x+y+coord.x+coord.y)%3;
                double n = noise.noise(
                double(x + coord.x * width * wx) * 0.05,
                double(y + coord.y * height * wy) * 0.05
            );

                float v = float((n + 1.0) * 0.5); // normalize 0–1
                tileColors[y*width+x] = Color(v,v * 2,v,1);
            }
        auto e = std::make_shared<Entity>(Vector2i(width/2,height/2),true);
        entities.push_back(e);
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
