#pragma once

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <memory>
#include <tuple>
#include <vector>

class World;
class Entity;

using namespace godot;

class Chunk {
public:
    World *world = nullptr;
    Vector2i coord;
    int width = 0;
    int height = 0;

    bool is_dirty = false; // for the save/load (dont write unless dirty)

    std::vector<int> tiles;
    std::vector<Color> tileColors;
    std::vector<std::shared_ptr<Entity>> entities;

    Chunk(int w, int h, Vector2i c, World *parent_world = nullptr);
    Chunk(int w, int h, Vector2i c);

    void generate(int wx, int wy);
    void simulate(float delta);
    void transfer_entities(std::vector<std::shared_ptr<Entity>> &entities, Vector2i direction);

    int get_tile(int local_x, int local_y) const;
    void set_tile(int local_x, int local_y, int value);
    std::vector<Color> get_tile_colors() const;

    std::vector<int> getAvailableDirs(Vector2i current_world, Vector2i current_size,
                                      std::vector<std::tuple<Vector2i, int>> neighbourChunks);
};
