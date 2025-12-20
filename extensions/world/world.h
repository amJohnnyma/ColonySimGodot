#pragma once
#include <cstdint>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "chunk.h"
#include "godot_cpp/variant/dictionary.hpp"

using namespace godot;

struct Vector2iHash {
    std::size_t operator()(const Vector2i &v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};




class World : public  Node2D{
    GDCLASS(World, Node2D)

    private:
        std::unordered_map<Vector2i, std::shared_ptr<Chunk>, Vector2iHash> chunks;
        int chunk_size = 16;
        int world_chunks_x = 0;
        int world_chunks_y = 0;
        uint64_t current_entity_id = 0;

    protected:
        static void _bind_methods();

    public:
        void init(int world_width_tiles, int world_height_tiles, int chunk_size_tiles);
        void update(const Vector2 &origin, int render_distance_chunks, float delta);
        
        int get_tile(int world_x, int world_y) const;
        void set_tile(int world_x, int world_y, int value);
        
        int get_chunk_size() const;
        int get_chunk_width() const;
        int get_chunk_height() const;
        int get_world_width_tiles() const;
        int get_world_height_tiles() const;
        
        Vector2i world_pos_to_chunk(const Vector2 &pos) const;
        Vector2i world_tile_to_chunk(int tile_x, int tile_y) const;
        Vector2i world_tile_to_local(int tile_x, int tile_y) const;
        bool is_valid_chunk(const Vector2i &coord) const;
        
        Array get_chunk_colors(const Vector2i &coord);
        int get_chunk_entity_count(const Vector2i &coord) const;
        Vector2i get_entity_position(const Vector2i &chunk_coord, int entity_index) const;
        int get_chunk_entity_capacity() const;
        
        
        // Get all chunks that need rendering (clamped + culled)
        TypedArray<Vector2i> get_visible_chunks(
            const Vector2 &cam_pos,
            const Vector2 &world_min,
            const Vector2 &world_max,
            int max_render_distance
        );
        
        // Batch get all visible entity positions in one call
        Dictionary get_visible_entities(
            const TypedArray<Vector2i> &chunk_coords,
            const Vector2 &cull_min,
            const Vector2 &cull_max,
            int max_entities
        );
        
        // Internal helpers
        std::shared_ptr<Chunk> get_chunk(const Vector2i &coord);
        std::shared_ptr<Chunk> load_chunk(const Vector2i &coord);
        void unload_chunk(const Vector2i &coord);
        int get_next_entity_id() {current_entity_id++; return (current_entity_id-1);};

        // The building system
        void place_building_in_chunk(const Vector2i &tile_coord, const int &building_type);
};
