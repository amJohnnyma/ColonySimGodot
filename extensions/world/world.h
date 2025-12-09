#pragma once
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <cmath>
#include "chunk.h"

using namespace godot;

struct Vector2iHash {
    std::size_t operator()(const Vector2i &v) const noexcept {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

class World : public Node2D {
    GDCLASS(World, Node2D)

public:
    int chunk_size = 16;
    int world_chunks_x = 0;
    int world_chunks_y = 0;

    std::unordered_map<Vector2i,std::shared_ptr<Chunk>,Vector2iHash> chunks;

    static void _bind_methods();

    // Initialization
    void init(int world_width_tiles, int world_height_tiles, int chunk_size_tiles);

    // World to chunk/local conversions
    Vector2i world_pos_to_chunk(const Vector2 &pos) const;
    Vector2i world_tile_to_chunk(int tile_x,int tile_y) const;
    Vector2i world_tile_to_local(int tile_x,int tile_y) const;

    // Validity check
    bool is_valid_chunk(const Vector2i &coord) const;



    // Chunk management
    std::shared_ptr<Chunk> get_chunk(const Vector2i &coord);
    std::shared_ptr<Chunk> load_chunk(const Vector2i &coord);
    void unload_chunk(const Vector2i &coord);
    int get_chunk_entity_count(const Vector2i &coord) const;
    Vector2i get_entity_position(const Vector2i &chunk_coord, int entity_index) const;
    int get_chunk_entity_capacity() const; // returns 100

    // Tile access
    int get_tile(int world_x,int world_y) const;
    void set_tile(int world_x,int world_y,int value);

    // Update/simulate chunks
    void update(const Vector2 &origin, int render_distance_chunks, float delta);

    // Getters for world/chunk dimensions
    int get_chunk_width() const;
    int get_chunk_height() const;
    int get_chunk_size() const;
    int get_world_width_tiles() const;
    int get_world_height_tiles() const;

    // Optional: utility for chunk tile colors
   Array get_chunk_colors(const Vector2i &coord);
};
