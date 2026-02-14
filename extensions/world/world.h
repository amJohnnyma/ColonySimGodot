#pragma once

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <cmath>
#include <cstdint>
#include <memory>
#include <queue>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ProxyManager.h"
#include "chunk.h"
#include "entity.h"
#include "entityDataReader.h"
#include "entityJob.h"
#include "threadpool.h"
#include "chunk_manager.h"
#include "hash_utils.h"

using namespace godot;


class World : public Node2D {
    GDCLASS(World, Node2D)

private:
    std::unique_ptr<ThreadPool> thread_pool;
    std::unordered_map<Vector2i, std::shared_ptr<Chunk>, Vector2iHash> unloaded_chunk_backup;
    std::unordered_map<Vector2i, std::shared_ptr<Chunk>, Vector2iHash> chunks;

    struct ChunkSimCache {
        std::vector<std::shared_ptr<Chunk>> full_sim;
        std::vector<std::shared_ptr<Chunk>> light_sim;
        std::unordered_set<Vector2i, Vector2iHash> needed;

        void clear() {
            full_sim.clear();
            light_sim.clear();
            needed.clear();
        }
    };
    ChunkSimCache sim_cache;

    int chunk_size = 16;
    int world_chunks_x = 0;
    int world_chunks_y = 0;
    uint64_t current_entity_id = 0;
    std::vector<EntityJob> availableJobs;
    EntityDataReader entityDataReader;
    std::queue<Vector2i> load_queue;
    std::unordered_set<Vector2i, Vector2iHash> queued_chunks;
    std::unordered_map<int, ProxyEntity> proxy_entities;
    double world_time = 0.0;
    ProxyManager proxy_manager;
    std::set<Vector2i> last_visible_chunks;

    bool track_entity_movement_per_second = false;
    float avg_proxy_tiles_moved_temp = 0.0f;
    float total_time_running = 0.0f;
    int tiles_moved_total_proxy = 0;
    float avg_fullsim_tiles_moved_temp = 0.0f;
    int tiles_moved_total_fullsim = 0;
    ChunkManager chunkManager;

    void init_thread_pool(size_t thread_count = 0);

public:
    std::vector<std::pair<Vector2i, std::shared_ptr<Entity>>> pendingEntityPlacements;
    std::shared_mutex chunks_mutex;
    std::mutex pending_mutex;

protected:
    static void _bind_methods();

public:
    void init(int world_width_tiles, int world_height_tiles, int chunk_size_tiles);
    void update(const Vector2 &origin, int max_render_distance_chunks, float delta, bool paused);
    float get_world_time() { return world_time; }

    int get_tiles_moved_total_fullsim() { return tiles_moved_total_fullsim; }
    void increment_tiles_moved_total_fullsim(int tiles) { tiles_moved_total_fullsim += tiles; }

    void set_track_entity_movement_per_second(bool flag) { track_entity_movement_per_second = flag; }
    bool is_tracking_entity_movement_per_second() { return track_entity_movement_per_second; }
    void toggle_track_entity_movement_per_second() { track_entity_movement_per_second = !track_entity_movement_per_second; }

    void update_proxies(double current_time);

    int get_tile(int world_x, int world_y) const;
    void set_tile(int world_x, int world_y, int value);

    void request_chunk(Vector2i c);
    void process_chunk_loading();

    int get_chunk_size() const;
    int get_chunk_width() const;
    int get_chunk_height() const;
    int get_world_width_tiles() const;
    int get_world_height_tiles() const;

    Vector2i world_pos_to_chunk(const Vector2 &pos) const;
    Vector2i world_tile_to_chunk(int tile_x, int tile_y) const;
    Vector2i world_tile_to_local(int tile_x, int tile_y) const;
    bool is_valid_chunk(const Vector2i &coord) const;
    Vector2i world_pos_to_tile(const Vector2 &pos) const;

    Array get_chunk_colors(const Vector2i &coord);
    int get_chunk_entity_count(const Vector2i &coord) const;
    Vector2i get_entity_position(const Vector2i &chunk_coord, int entity_index) const;
    int get_chunk_entity_capacity() const;
    Dictionary get_entities_at_world_pos(const Vector2 coord);

    void flush_pending_placements();

    TypedArray<Vector2i> get_visible_chunks(
        const Vector2 &cam_pos,
        const Vector2 &world_min,
        const Vector2 &world_max,
        int max_render_distance
    );

    Dictionary get_visible_entities(
        const TypedArray<Vector2i> &chunk_coords,
        const Vector2 &cull_min,
        const Vector2 &cull_max,
        int max_entities
    );

    std::shared_ptr<Chunk> get_chunk(const Vector2i &coord);
    std::shared_ptr<Chunk> load_chunk(const Vector2i &coord);
    void unload_chunk(const Vector2i &coord);
    int get_next_entity_id() { current_entity_id++; return (current_entity_id - 1); }

    void create_entity(const String &type, const Vector2i &coord, const int &entity_type, const int &entity_sprite);
    void create_temp_job(const Vector2i &jobPos, const Vector2i &entityPos, const int &id, const int &jobType);
    EntityJob create_job(const int &entityType, const int &jobType, const Vector2i &jobPos);
    bool test_serialization(); 
};
