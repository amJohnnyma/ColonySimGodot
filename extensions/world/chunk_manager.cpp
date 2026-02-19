#include "chunk_manager.h"
#include "chunk.h"
#include "entity.h"
#include "entityJob.h"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "world.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

ChunkManager::ChunkManager() : world_save_path("") {
//    thread_pool = WorkerThreadPool::get_singleton();
     //   thread_pool = nullptr;
}

ChunkManager::~ChunkManager() {
    shutdown();
}

// Helper to recursively remove directories
static Error remove_directory_recursive(const String& path) {
    if (!DirAccess::dir_exists_absolute(path)) {
        return OK;
    }

    Ref<DirAccess> dir = DirAccess::open(path);
    if (dir.is_null()) {
        return ERR_CANT_OPEN;
    }

    dir->list_dir_begin();
    String item = dir->get_next();
    while (!item.is_empty()) {
        if (item == "." || item == "..") {
            item = dir->get_next();
            continue;
        }

        String full_path = path.path_join(item);

        if (dir->current_is_dir()) {
            Error sub_err = remove_directory_recursive(full_path);
            if (sub_err != OK) {
                return sub_err;
            }
        } else {
            Error file_err = DirAccess::remove_absolute(full_path);
            if (file_err != OK) {
                UtilityFunctions::printerr(vformat("Failed to delete file: %s (Error %d)", full_path, file_err));
                return file_err;
            }
        }

        item = dir->get_next();
    }
    dir->list_dir_end();

    Error final_err = DirAccess::remove_absolute(path);
    if (final_err != OK) {
        UtilityFunctions::printerr(vformat("Failed to delete directory: %s (Error %d)", path, final_err));
    }
    return final_err;
}

void ChunkManager::initialize(const String& world_name, int load_rad, int unload_rad, 
                              int chunk_w, int chunk_h, World* world_ptr) {
    load_radius = load_rad;
    unload_radius = unload_rad;
    chunk_width = chunk_w;
    chunk_height = chunk_h;
    world = world_ptr;

    // Get world dimensions from World class
    int world_width_tiles = world_ptr->get_world_width_tiles();    
    int world_height_tiles = world_ptr->get_world_width_tiles();
    
    // Calculate max chunk coordinates (0-indexed)
    max_chunk_x = (world_width_tiles / chunk_width) - 1;   // 512/64 - 1 = 7
    max_chunk_y = (world_height_tiles / chunk_height) - 1; // 512/64 - 1 = 7
    
    UtilityFunctions::print(vformat("World bounds: chunks (0,0) to (%d,%d)", max_chunk_x, max_chunk_y));

    String fixed_world_name = "NewWorld";
    world_save_path = "user://worlds/" + fixed_world_name + "/chunks/";

    // Delete existing world folder
    String parent_dir = "user://worlds/" + fixed_world_name + "/";
    Error del_err = remove_directory_recursive(parent_dir);
    if (del_err != OK) {
        UtilityFunctions::printerr(vformat("Failed to fully delete old world '%s' (Error %d). Continuing anyway.", 
                                          fixed_world_name, del_err));
    } else if (DirAccess::dir_exists_absolute(parent_dir)) {
        UtilityFunctions::print("Old world folder deleted successfully.");
    } else {
        UtilityFunctions::print("No old world folder found (clean start).");
    }

    // Recreate the chunks directory
    Error create_err = DirAccess::make_dir_recursive_absolute(world_save_path);
    if (create_err != OK) {
        UtilityFunctions::printerr(vformat("Failed to create chunk save directory: %s (Error %d)", 
                                          world_save_path, create_err));
        return;
    }

    io_thread_should_exit = false;
    io_thread = std::thread(&ChunkManager::_io_worker, this);
}

void ChunkManager::shutdown() {
    io_thread_should_exit = true;
    if (io_thread.joinable()) {
        io_thread.join();
    }

    save_all_dirty_chunks_now();
    
    // Clean up loaded chunks
    std::lock_guard<std::mutex> lock(chunks_mutex);
    for (auto& pair : loaded_chunks) {
        delete pair.second;
    }
    loaded_chunks.clear();
}

String ChunkManager::_get_chunk_path(const Vector2i& coord) const {
    return world_save_path + vformat("c_%d_%d.bin", coord.x, coord.y);
}

PackedByteArray ChunkManager::_serialize_chunk(const Chunk* chunk) const {
    PackedByteArray data;
    
    UtilityFunctions::print("=== SERIALIZATION START ===");
    
    // Version (4 bytes)
    data.resize(4);
    data.encode_u32(0, 1);
    UtilityFunctions::print("Wrote version: 1");
    
    // Chunk coordinates (8 bytes)
    PackedInt32Array coord_data;
    coord_data.push_back(chunk->coord.x);
    coord_data.push_back(chunk->coord.y);
    data.append_array(coord_data.to_byte_array());
    UtilityFunctions::print("Wrote chunk coord: (", chunk->coord.x, ", ", chunk->coord.y, ")");
    
    // Width and height (8 bytes)
    PackedInt32Array size_data;
    size_data.push_back(chunk->width);
    size_data.push_back(chunk->height);
    data.append_array(size_data.to_byte_array());
    UtilityFunctions::print("Wrote chunk size: ", chunk->width, "x", chunk->height);
    
    // Tiles count and data
    uint32_t tile_count = chunk->tiles.size();
    PackedInt32Array tiles_header;
    tiles_header.push_back(tile_count);
    data.append_array(tiles_header.to_byte_array());
    UtilityFunctions::print("Wrote tile count: ", tile_count);
    
    for (int tile : chunk->tiles) {
        PackedInt32Array tile_data;
        tile_data.push_back(tile);
        data.append_array(tile_data.to_byte_array());
    }
    
    // Tile colors
    for (const Color& color : chunk->tileColors) {
        PackedFloat32Array color_data;
        color_data.push_back(color.r);
        color_data.push_back(color.g);
        color_data.push_back(color.b);
        color_data.push_back(color.a);
        data.append_array(color_data.to_byte_array());
    }
    UtilityFunctions::print("Wrote ", chunk->tileColors.size(), " tile colors");
    
    // Entity count
    uint32_t entity_count = chunk->entities.size();
    PackedInt32Array entity_header;
    entity_header.push_back(entity_count);
    data.append_array(entity_header.to_byte_array());
    UtilityFunctions::print("Wrote entity count: ", entity_count);
    
    // Entities
    int entity_idx = 0;
    
    for (const auto& e : chunk->entities) {
        if (!e) continue;
        
        UtilityFunctions::print("  Entity ", entity_idx, ":");
        
        // Entity type ID
        int entity_type_id = e->get_type_id();
        PackedInt32Array type_data;
        type_data.push_back(entity_type_id);
        data.append_array(type_data.to_byte_array());
        UtilityFunctions::print("    Type ID: ", entity_type_id);
        
        // Entity ID
        PackedInt64Array id_data;
        id_data.push_back(e->get_entity_id());
        data.append_array(id_data.to_byte_array());
        UtilityFunctions::print("    Entity ID: ", e->get_entity_id());
        
        // Position
        PackedInt32Array pos_data;
        pos_data.push_back(e->get_position().x);
        pos_data.push_back(e->get_position().y);
        data.append_array(pos_data.to_byte_array());
        UtilityFunctions::print("    Position: (", e->get_position().x, ", ", e->get_position().y, ")");
        
        // Size
        PackedInt32Array size_data;
        size_data.push_back(e->get_entity_size().x);
        size_data.push_back(e->get_entity_size().y);
        data.append_array(size_data.to_byte_array());
        UtilityFunctions::print("    Size: (", e->get_entity_size().x, ", ", e->get_entity_size().y, ")");
        
        // Sprite
        PackedInt32Array sprite_data;
        sprite_data.push_back(e->get_entity_sprite());
        data.append_array(sprite_data.to_byte_array());
        UtilityFunctions::print("    Sprite: ", e->get_entity_sprite());
        
        // Movement data
        PackedFloat32Array movement_data;
        movement_data.push_back(e->get_move_speed());
        movement_data.push_back(e->get_base_move_speed());
        data.append_array(movement_data.to_byte_array());
        UtilityFunctions::print("    Move speed: ", e->get_move_speed(), ", Base: ", e->get_base_move_speed());
        
        // Flags
        PackedInt32Array flags_data;
        flags_data.push_back(e->is_active() ? 1 : 0);
        flags_data.push_back(e->is_must_simulate() ? 1 : 0);
        data.append_array(flags_data.to_byte_array());
        UtilityFunctions::print("    Active: ", e->is_active(), ", Must simulate: ", e->is_must_simulate());
        
        // Job data
        auto jobs = e->get_job_list();
        PackedInt32Array job_count_data;
        job_count_data.push_back(jobs.size());
        data.append_array(job_count_data.to_byte_array());
        UtilityFunctions::print("    Job count: ", jobs.size());
        
        PackedInt32Array job_index_data;
        job_index_data.push_back(e->get_current_job_index());
        data.append_array(job_index_data.to_byte_array());
        UtilityFunctions::print("    Current job index: ", e->get_current_job_index());
        
        // Serialize jobs
        int job_idx = 0;
        for (auto& j : jobs) {
            UtilityFunctions::print("      Job ", job_idx, ":");
            
            PackedInt32Array job_flags_data;
            job_flags_data.push_back(j.isValid ? 1 : 0);
            job_flags_data.push_back(j.complete ? 1 : 0);
            data.append_array(job_flags_data.to_byte_array());
            UtilityFunctions::print("        Valid: ", j.isValid, ", Complete: ", j.complete);

            PackedInt32Array job_target_coord;
            job_target_coord.push_back(j.target_coord.x);
            job_target_coord.push_back(j.target_coord.y);
            data.append_array(job_target_coord.to_byte_array());
            UtilityFunctions::print("        Target: (", j.target_coord.x, ", ", j.target_coord.y, ")");

            const std::string& algo = j.move_algo;
            PackedInt32Array len;
            len.push_back(static_cast<int32_t>(algo.size()));
            data.append_array(len.to_byte_array());
            PackedByteArray algo_bytes;
            algo_bytes.resize(static_cast<int32_t>(algo.size()));
            uint8_t* ptr = algo_bytes.ptrw();
            std::memcpy(ptr, algo.data(), algo.size());
            data.append_array(algo_bytes);
            UtilityFunctions::print("        Algo: '", String(algo.c_str()), "' (", algo.size(), " bytes)");

            PackedInt32Array prio_data;
            prio_data.push_back(j.priority);
            data.append_array(prio_data.to_byte_array());
            UtilityFunctions::print("        Priority: ", j.priority);

            PackedFloat32Array moveSpeedMultiplier_data;
            moveSpeedMultiplier_data.push_back(j.moveSpeedMultiplier);
            data.append_array(moveSpeedMultiplier_data.to_byte_array());
            UtilityFunctions::print("        Speed multiplier: ", j.moveSpeedMultiplier);
            
            job_idx++;
        }

        // Type-specific data
        switch(entity_type_id) {
            case 1:
                if(auto colonist = std::dynamic_pointer_cast<Colonist>(e)) {
                    PackedInt32Array homeCoord;
                    homeCoord.push_back(colonist->get_home_coord().x);
                    homeCoord.push_back(colonist->get_home_coord().y);
                    data.append_array(homeCoord.to_byte_array());
                    UtilityFunctions::print("    Colonist home: (", colonist->get_home_coord().x, ", ", colonist->get_home_coord().y, ")");
                }
                break;
            case 2:
            case 3:
            default:
                break;
        }
        
        entity_idx++;
    }
    
    UtilityFunctions::print("=== SERIALIZATION END - Total bytes: ", data.size(), " ===");
    return data;
}

Chunk* ChunkManager::_deserialize_chunk(const PackedByteArray& raw) const {
    if (raw.is_empty()) return nullptr;
    
    UtilityFunctions::print("=== DESERIALIZATION START - Reading ", raw.size(), " bytes ===");
    
    int offset = 0;
    
    // Version
    uint32_t version = raw.decode_u32(offset);
    offset += 4;
    UtilityFunctions::print("Read version: ", version);
    if (version != 1) {
        UtilityFunctions::printerr("Invalid chunk version");
        return nullptr;
    }
    
    // Chunk coordinates
    int32_t coord_x = raw.decode_s32(offset);
    offset += 4;
    int32_t coord_y = raw.decode_s32(offset);
    offset += 4;
    Vector2i coord(coord_x, coord_y);
    UtilityFunctions::print("Read chunk coord: (", coord_x, ", ", coord_y, ")");
    
    // Width and height
    int32_t width = raw.decode_s32(offset);
    offset += 4;
    int32_t height = raw.decode_s32(offset);
    offset += 4;
    UtilityFunctions::print("Read chunk size: ", width, "x", height);
    
    // Create chunk
    Chunk* chunk = new Chunk(width, height, coord, world);
    
    // Tiles
    uint32_t tile_count = raw.decode_u32(offset);
    offset += 4;
    UtilityFunctions::print("Read tile count: ", tile_count);
    
    chunk->tiles.clear();
    chunk->tiles.reserve(tile_count);
    for (uint32_t i = 0; i < tile_count; ++i) {
        int32_t tile = raw.decode_s32(offset);
        offset += 4;
        chunk->tiles.push_back(tile);
    }
    
    // Tile colors
    chunk->tileColors.clear();
    chunk->tileColors.reserve(tile_count);
    for (uint32_t i = 0; i < tile_count; ++i) {
        float r = raw.decode_float(offset); offset += 4;
        float g = raw.decode_float(offset); offset += 4;
        float b = raw.decode_float(offset); offset += 4;
        float a = raw.decode_float(offset); offset += 4;
        chunk->tileColors.push_back(Color(r, g, b, a));
    }
    UtilityFunctions::print("Read ", chunk->tileColors.size(), " tile colors");
    
    // Entity count
    uint32_t entity_count = raw.decode_u32(offset);
    offset += 4;
    UtilityFunctions::print("Read entity count: ", entity_count);
    
    chunk->entities.clear();
    chunk->entities.reserve(entity_count);

    // Deserialize entities
    for (uint32_t i = 0; i < entity_count; ++i) {
        UtilityFunctions::print("  Entity ", i, ":");
        
        int32_t type_id = raw.decode_u32(offset);
        offset += 4;
        UtilityFunctions::print("    Type ID: ", type_id);
        
        int64_t entity_id = raw.decode_s64(offset);
        offset += 8;
        UtilityFunctions::print("    Entity ID: ", entity_id);
        
        int32_t pos_x = raw.decode_s32(offset);
        offset += 4;
        int32_t pos_y = raw.decode_s32(offset);
        offset += 4;
        UtilityFunctions::print("    Position: (", pos_x, ", ", pos_y, ")");
        
        int32_t size_x = raw.decode_s32(offset);
        offset += 4;
        int32_t size_y = raw.decode_s32(offset);
        offset += 4;
        UtilityFunctions::print("    Size: (", size_x, ", ", size_y, ")");
        
        int32_t sprite_idx = raw.decode_s32(offset);
        offset += 4;
        UtilityFunctions::print("    Sprite: ", sprite_idx);
        
        float move_speed = raw.decode_float(offset);
        offset += 4;
        float base_move_speed = raw.decode_float(offset);
        offset += 4;
        UtilityFunctions::print("    Move speed: ", move_speed, ", Base: ", base_move_speed);
        
        bool is_active = raw.decode_s32(offset);
        offset += 4;
        bool is_must_simulate = raw.decode_s32(offset);
        offset += 4;
        UtilityFunctions::print("    Active: ", is_active, ", Must simulate: ", is_must_simulate);
        
        uint32_t job_count = raw.decode_u32(offset);
        offset += 4;
        UtilityFunctions::print("    Job count: ", job_count);
        
        int32_t job_index = raw.decode_s32(offset);
        offset += 4;
        UtilityFunctions::print("    Current job index: ", job_index);
        
        std::vector<EntityJob> jobList;
        jobList.reserve(job_count);
        
        // Deserialize jobs
        for (uint32_t j = 0; j < job_count; ++j) {
            UtilityFunctions::print("      Job ", j, ":");
            
            bool is_valid = raw.decode_s32(offset);
            offset += 4;
            bool is_complete = raw.decode_s32(offset);
            offset += 4;
            UtilityFunctions::print("        Valid: ", is_valid, ", Complete: ", is_complete);
            
            int32_t target_coord_x = raw.decode_s32(offset);
            offset += 4;
            int32_t target_coord_y = raw.decode_s32(offset);
            offset += 4;
            UtilityFunctions::print("        Target: (", target_coord_x, ", ", target_coord_y, ")");
            
            int32_t len = raw.decode_s32(offset);
            offset += 4;
            std::string algo;
            algo.resize(len);
            std::memcpy(algo.data(), &raw[offset], len);
            offset += len;
            UtilityFunctions::print("        Algo: '", String(algo.c_str()), "' (", len, " bytes)");
            
            uint32_t priority = raw.decode_s32(offset);
            offset += 4;
            UtilityFunctions::print("        Priority: ", priority);
            
            float move_speed_multiplier = raw.decode_float(offset);
            offset += 4;
            UtilityFunctions::print("        Speed multiplier: ", move_speed_multiplier);

            EntityJob job;
            job.isValid = is_valid;
            job.complete = is_complete;
            job.target_coord = Vector2i(target_coord_x, target_coord_y);
            job.move_algo = algo;
            job.priority = priority;
            job.moveSpeedMultiplier = move_speed_multiplier;

            jobList.push_back(job);
        }

        std::shared_ptr<Entity> entity;
        
        // Create entity based on type
        switch(type_id) {
            case 1: {
                uint32_t home_x = raw.decode_s32(offset);
                offset += 4;
                uint32_t home_y = raw.decode_s32(offset);
                offset += 4;
                UtilityFunctions::print("    Colonist home: (", home_x, ", ", home_y, ")");

                auto colonist = std::make_shared<Colonist>();
                colonist->set_home_coord(Vector2i(home_x, home_y));
                entity = colonist;
                break;
            }
            case 2: {
                auto building = std::make_shared<Building>();
                entity = building;
                break;
            }
            case 3: {
                auto item = std::make_shared<Item>();
                entity = item;
                break;
            }
            default:
                UtilityFunctions::printerr("Unknown entity type: ", type_id);
                break;
        }
        
        if (!entity) {
            UtilityFunctions::printerr("Failed to create entity of type ", type_id);
            continue;
        }
        
        entity->set_entity_id(entity_id);
        entity->set_entity_sprite(sprite_idx);
        entity->set_entity_size(Vector2i(size_x, size_y));
        entity->set_active(is_active);
        entity->set_position(Vector2i(pos_x, pos_y));
        entity->set_move_speed(move_speed);
        entity->set_base_move_speed(base_move_speed);
        entity->set_must_simulate(is_must_simulate);
        entity->set_current_job_index(job_index);
        entity->set_job_list(jobList);
        
        chunk->entities.push_back(entity);
        UtilityFunctions::print("    Entity added to chunk!");
    }
    
    chunk->is_dirty = false;
    UtilityFunctions::print("=== DESERIALIZATION END ===");
    return chunk;
}

// IO WORKER
void ChunkManager::_io_worker() {
    while (!io_thread_should_exit) {
        IO_Task task{};
        bool has_task = false;

        {
            std::lock_guard<std::mutex> lock(io_mutex);
            if (!io_queue.empty()) {
                task = io_queue.front();
                io_queue.pop();
                has_task = true;
            }
        }

        if (!has_task) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        switch (task.type) {
            case IO_Task::SAVE: {
                // Just write the pre-serialized data
                Ref<FileAccess> f = FileAccess::open(task.file_path, FileAccess::WRITE);
                if (f.is_valid()) {
                    f->store_buffer(task.raw_data);
                }
                break;
            }
            case IO_Task::LOAD: {
                // Just read raw bytes
                PackedByteArray raw;
                Ref<FileAccess> f = FileAccess::open(task.file_path, FileAccess::READ);
                if (f.is_valid() && f->get_length() > 0) {
                    raw = f->get_buffer(f->get_length());
                }
                
                // Callback will handle deserialization on main thread
                if (task.load_callback) {
                    // Need to invoke this on main thread!
                    // Store for processing in update()
                    std::lock_guard<std::mutex> lock(completed_loads_mutex);
                    completed_loads.push_back({task.load_callback, raw});
                }
                break;
            }
            case IO_Task::DELETE: {
                DirAccess::remove_absolute(task.file_path);
                break;
            }
        }
    }
}

void ChunkManager::_queue_save(Chunk* chunk) {
    if (!chunk) return;

    // Don't save negative chunks
    if (!_is_valid_chunk_coord(chunk->coord)) {
        chunk->is_dirty = false;  // Mark as clean so we don't try again
        return;
    }
    
    // SERIALIZE ON MAIN THREAD
    PackedByteArray data = _serialize_chunk(chunk);
    
    IO_Task task;
    task.type = IO_Task::SAVE;
    task.coord = chunk->coord;
    task.raw_data = data;  // Pass serialized data
    task.file_path = _get_chunk_path(chunk->coord);
    
    chunk->is_dirty = false;
    
    std::lock_guard<std::mutex> lock(io_mutex);
    io_queue.push(task);
}

// Modified _queue_load
void ChunkManager::_queue_load(const Vector2i& coord, std::function<void(Chunk*)> callback) {
    IO_Task task;
    task.type = IO_Task::LOAD;
    task.coord = coord;
    task.file_path = _get_chunk_path(coord);
    
    // Load callback now receives raw data
    task.load_callback = [this, coord, callback](PackedByteArray raw_data) {
        // DESERIALIZE ON MAIN THREAD
        Chunk* chunk = nullptr;
        if (!raw_data.is_empty()) {
            chunk = _deserialize_chunk(raw_data);
        }
        
        {
            std::lock_guard<std::mutex> lock(loading_mutex);
            chunks_being_loaded.erase(coord);
        }
        
        if (chunk) {
            std::lock_guard<std::mutex> lock(chunks_mutex);
            loaded_chunks[coord] = chunk;
        }
        
        if (callback) {
            callback(chunk);
        }
    };
    
    std::lock_guard<std::mutex> lock(io_mutex);
    io_queue.push(task);
}
void ChunkManager::_handle_load_complete(Vector2i coord, Chunk* chunk, std::function<void(Chunk*)> callback) {
    if (chunk) {
        std::lock_guard<std::mutex> lock(chunks_mutex);
        loaded_chunks[coord] = chunk;
    }
    if (callback) {
        callback(chunk);
    }
}

// PUBLIC API

Chunk* ChunkManager::get_chunk(const Vector2i& coord, bool allow_generate_if_missing) {
    if (!_is_valid_chunk_coord(coord)) {
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(chunks_mutex);
        auto it = loaded_chunks.find(coord);
        if (it != loaded_chunks.end()) {
            return it->second;
        }
    }

    // Check if already loading
    {
        std::lock_guard<std::mutex> lock(loading_mutex);
        if (chunks_being_loaded.find(coord) != chunks_being_loaded.end()) {
            return nullptr; // Already loading
        }
        chunks_being_loaded.insert(coord);
    }

    // Queue async load
    _queue_load(coord, [this, coord, allow_generate_if_missing](Chunk* chunk) {
        {
            std::lock_guard<std::mutex> lock(loading_mutex);
            chunks_being_loaded.erase(coord);
        }
        
        if (!chunk && allow_generate_if_missing) {
            // Generate new chunk
            chunk = new Chunk(chunk_width, chunk_height, coord, world);
            chunk->generate(coord.x * chunk_width, coord.y * chunk_height);
            chunk->is_dirty = true;
        }
        
        if (chunk) {
            std::lock_guard<std::mutex> lock(chunks_mutex);
            loaded_chunks[coord] = chunk;
        }
    });

    return nullptr; // Will be available next frame
}

void ChunkManager::unload_chunk(const Vector2i& coord) {
    Chunk* chunk = nullptr;
    {
        std::lock_guard<std::mutex> lock(chunks_mutex);
        auto it = loaded_chunks.find(coord);
        if (it != loaded_chunks.end()) {
            chunk = it->second;
            loaded_chunks.erase(it);
        }
    }
    
    if (chunk) {
        if (chunk->is_dirty) {
            _queue_save(chunk);
        }
        delete chunk;
    }
}

void ChunkManager::mark_dirty(Chunk* chunk) {
    if (chunk && !chunk->is_dirty) {
        chunk->is_dirty = true;
    }
}

void ChunkManager::save_chunk_now(Chunk* chunk) {
    if (!chunk || !chunk->is_dirty) return;
    
    PackedByteArray data = _serialize_chunk(chunk);
    Ref<FileAccess> f = FileAccess::open(_get_chunk_path(chunk->coord), FileAccess::WRITE);
    if (f.is_valid()) {
        f->store_buffer(data);
        chunk->is_dirty = false;
    }
}

void ChunkManager::save_all_dirty_chunks_now() {
    std::vector<Vector2i> coords;
    {
        std::lock_guard<std::mutex> lock(chunks_mutex);
        for (const auto& pair : loaded_chunks) {
            coords.push_back(pair.first);
        }
    }
    
    for (const Vector2i& c : coords) {
        Chunk* ch = nullptr;
        {
            std::lock_guard<std::mutex> lock(chunks_mutex);
            auto it = loaded_chunks.find(c);
            if (it != loaded_chunks.end()) {
                ch = it->second;
            }
        }
        if (ch && ch->is_dirty) {
            save_chunk_now(ch);
        }
    }
}

void ChunkManager::update(const Vector2& player_world_pos) {
    Vector2i player_chunk = {
        (int)std::floor(player_world_pos.x / chunk_width),
        (int)std::floor(player_world_pos.y / chunk_height)
    };

    // Load chunks in radius
    for (int dy = -load_radius; dy <= load_radius; ++dy) {
        for (int dx = -load_radius; dx <= load_radius; ++dx) {
            Vector2i c = { player_chunk.x + dx, player_chunk.y + dy };
            if (!_is_valid_chunk_coord(c)) {
                continue; // Skip invalid chunks
            }
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist <= load_radius) {
                get_chunk(c, true); // Async load/generate
            }
        }
    }

    // Unload far chunks
    std::vector<Vector2i> to_unload;
    {
        std::lock_guard<std::mutex> lock(chunks_mutex);
        for (const auto& pair : loaded_chunks) {
            int dx = pair.first.x - player_chunk.x;
            int dy = pair.first.y - player_chunk.y;
            if (std::sqrt(dx*dx + dy*dy) > unload_radius) {
                to_unload.push_back(pair.first);
            }
        }
    }
    
    for (const Vector2i& c : to_unload) {
        unload_chunk(c);
    }
}
void ChunkManager::process_completed_loads() {
    std::vector<std::pair<std::function<void(PackedByteArray)>, PackedByteArray>> to_process;
    
    {
        std::lock_guard<std::mutex> lock(completed_loads_mutex);
        to_process.swap(completed_loads);
    }
    
    for (auto& pair : to_process) {
        pair.first(pair.second);  // Execute callback on main thread
    }
}


bool ChunkManager::test_serialize_deserialize() {
    UtilityFunctions::print("\n=== TEST: Chunk Serialization ===");
    
    // Create a test chunk
    Chunk* original = new Chunk(16, 16, Vector2i(5, 10), nullptr);
    
    // Add test data
    original->tiles = {1, 2, 3, 4, 5};
    original->tileColors = {
        Color(1.0f, 0.0f, 0.0f, 1.0f),
        Color(0.0f, 1.0f, 0.0f, 1.0f),
        Color(0.0f, 0.0f, 1.0f, 1.0f),
        Color(1.0f, 1.0f, 0.0f, 1.0f),
        Color(1.0f, 0.0f, 1.0f, 1.0f)
    };
    
    // Create colonist with jobt
    auto colonist = std::make_shared<Colonist>();
    colonist->set_entity_id(12345);
    colonist->set_type_id(1);
    colonist->set_position(Vector2i(100, 200));
    colonist->set_entity_size(Vector2i(1, 1));
    colonist->set_entity_sprite(42);
    colonist->set_move_speed(5.5f);
    colonist->set_base_move_speed(5.0f);
    colonist->set_active(true);
    colonist->set_must_simulate(true);
    colonist->set_home_coord(Vector2i(50, 60));
    colonist->set_current_job_index(0);
    
    // Add jobs
    EntityJob job1;
    job1.isValid = true;
    job1.complete = false;
    job1.target_coord = Vector2i(150, 250);
    job1.move_algo = "A*";
    job1.priority = 10;
    job1.moveSpeedMultiplier = 1.2f;
    
    EntityJob job2;
    job2.isValid = true;
    job2.complete = true;
    job2.target_coord = Vector2i(75, 125);
    job2.move_algo = "dijkstra";
    job2.priority = 5;
    job2.moveSpeedMultiplier = 0.8f;
    
    std::vector<EntityJob> jobs = {job1, job2};
    colonist->set_job_list(jobs);
    
    original->entities.push_back(colonist);
    
    UtilityFunctions::print("Original entity count: ", original->entities.size());
    UtilityFunctions::print("Original job count: ", colonist->get_job_list().size());
    
    // Serialize
    PackedByteArray data = _serialize_chunk(original);
    UtilityFunctions::print("Serialized size: ", data.size(), " bytes");
    
    // Deserialize
    Chunk* deserialized = _deserialize_chunk(data);
    
    if (!deserialized) {
        UtilityFunctions::printerr("Deserialization failed!");
        delete original;
        return false;
    }
    
    UtilityFunctions::print("Deserialized entity count: ", deserialized->entities.size());
    if (deserialized->entities.size() > 0) {
        UtilityFunctions::print("Deserialized job count: ", deserialized->entities[0]->get_job_list().size());
        
        // Check if jobs match
        auto orig_jobs = original->entities[0]->get_job_list();
        auto deser_jobs = deserialized->entities[0]->get_job_list();
        
        if (orig_jobs.size() == deser_jobs.size()) {
            for (size_t i = 0; i < orig_jobs.size(); i++) {
                if (orig_jobs[i].move_algo != deser_jobs[i].move_algo) {
                    UtilityFunctions::printerr("Job algorithm mismatch!");
                    delete original;
                    delete deserialized;
                    return false;
                }
            }
        } else {
            UtilityFunctions::printerr("Job count mismatch!");
            delete original;
            delete deserialized;
            return false;
        }
    }
    
    // Simple checks
    bool passed = true;
    
    if (original->entities.size() != deserialized->entities.size()) {
        UtilityFunctions::printerr("Entity count mismatch!");
        passed = false;
    }
    
    if (original->tiles.size() != deserialized->tiles.size()) {
        UtilityFunctions::printerr("Tile count mismatch!");
        passed = false;
    }
    
    delete original;
    delete deserialized;
    
    UtilityFunctions::print(passed ? "✓ PASSED" : "✗ FAILED");
    return passed;
}
