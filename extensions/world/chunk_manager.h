#pragma once

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>

#include "hash_utils.h"

using namespace godot;

class Chunk;
class World;

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void initialize(const String& world_name, int load_rad, int unload_rad, 
                   int chunk_w, int chunk_h, World* world_ptr);
    void shutdown();

    // Get chunk (may return nullptr if still loading)
    Chunk* get_chunk(const Vector2i& coord, bool allow_generate_if_missing);
    
    void unload_chunk(const Vector2i& coord);
    void mark_dirty(Chunk* chunk);
    void save_chunk_now(Chunk* chunk);
    void save_all_dirty_chunks_now();
    
    // Called each frame to update loading/unloading
    void update(const Vector2& player_world_pos);

private:
    struct IO_Task {
        enum Type { LOAD, SAVE, DELETE };
        Type type;
        Vector2i coord;
        Chunk* chunk = nullptr;
        String file_path;
        std::function<void(Chunk*)> load_callback;
    };

    // Thread-safe IO queue
    std::queue<IO_Task> io_queue;
    std::mutex io_mutex;
    std::thread io_thread;
    std::atomic<bool> io_thread_should_exit{false};

    // Loaded chunks
    Vector2iMap<Chunk*> loaded_chunks;
    std::mutex chunks_mutex;

    // Track chunks currently being loaded to avoid duplicate requests
    Vector2iSet chunks_being_loaded;
    std::mutex loading_mutex;

    // Config
    int load_radius = 8;
    int unload_radius = 12;
    int chunk_width = 16;
    int chunk_height = 16;
    World* world = nullptr;
    String world_save_path;

    WorkerThreadPool* thread_pool = nullptr;

    // Helpers
    String _get_chunk_path(const Vector2i& coord) const;
    PackedByteArray _serialize_chunk(const Chunk* chunk) const;
    Chunk* _deserialize_chunk(const PackedByteArray& raw) const;
    
    void _io_worker();
    void _queue_load(const Vector2i& coord, std::function<void(Chunk*)> callback);
    void _queue_save(Chunk* chunk);
    void _handle_load_complete(Vector2i coord, Chunk* chunk, std::function<void(Chunk*)> callback);
};
