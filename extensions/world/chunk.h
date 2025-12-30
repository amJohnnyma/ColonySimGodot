// chunk.h
#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <memory>
#include <mutex>
#include <future>
#include <vector>
#include "entity.h"
#include "colonist.h"
#include "building.h"
#include "item.h"

class World;  // Forward declaration — NO CIRCULAR INCLUDE!

using namespace godot;


class Chunk : public std::enable_shared_from_this<Chunk> {
    public:
        World* world = nullptr;
        Vector2i coord;
        int width = 0;
        int height = 0;

        std::vector<int> tiles;
        std::vector<Color> tileColors;
        std::vector<std::shared_ptr<Entity>> entities;

        // Constructors
        Chunk(int w, int h, Vector2i c, World* parent_world = nullptr);
        Chunk(int w, int h, Vector2i c);  // Legacy support

        // Methods
        void generate(int wx, int wy);
        void simulate(float delta, bool full_simulation = true);
        void transfer_entities(std::vector<std::shared_ptr<Entity>>& entities, Vector2i direction);

        int get_tile(int local_x, int local_y) const;
        void set_tile(int local_x, int local_y, int value);
        std::vector<Color> get_tile_colors() const;
        
        std::vector<int> getAvailableDirs(Vector2i current, std::vector<std::tuple<Vector2i, int>> neighbourChunks);
};
