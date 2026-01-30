#include "world.h"
#include "chunk.h"
#include "core/math.hpp"
#include "entityDataReader.h"
#include "entityJob.h"
#include "variant/vector2i.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>
#include <unordered_set>

using namespace godot;


void World::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "world_width_tiles", "world_height_tiles", "chunk_size_tiles"),
            &World::init);
    ClassDB::bind_method(D_METHOD("update", "origin", "max_render_distance_chunks", "simulation_distance", "delta", "paused"),
            &World::update);
    ClassDB::bind_method(D_METHOD("get_tile", "world_x", "world_y"), &World::get_tile);
    ClassDB::bind_method(D_METHOD("set_tile", "world_x", "world_y", "value"), &World::set_tile);
    ClassDB::bind_method(D_METHOD("get_chunk_size"), &World::get_chunk_size);
    ClassDB::bind_method(D_METHOD("get_chunk_width"), &World::get_chunk_width);
    ClassDB::bind_method(D_METHOD("get_chunk_height"), &World::get_chunk_height);
    ClassDB::bind_method(D_METHOD("get_world_width_tiles"), &World::get_world_width_tiles);
    ClassDB::bind_method(D_METHOD("get_world_height_tiles"), &World::get_world_height_tiles);
    ClassDB::bind_method(D_METHOD("world_pos_to_chunk", "pos"), &World::world_pos_to_chunk);
    ClassDB::bind_method(D_METHOD("is_valid_chunk", "coord"), &World::is_valid_chunk);
    ClassDB::bind_method(D_METHOD("get_chunk_colors", "coord"), &World::get_chunk_colors);
    ClassDB::bind_method(D_METHOD("get_chunk_entity_count", "coord"), &World::get_chunk_entity_count);
    ClassDB::bind_method(D_METHOD("get_entity_position", "chunk_coord", "entity_index"), &World::get_entity_position);
    ClassDB::bind_method(D_METHOD("get_chunk_entity_capacity"), &World::get_chunk_entity_capacity);
    ClassDB::bind_method(D_METHOD("get_visible_chunks", "cam_pos", "world_min", "world_max", "max_render_distance"),&World::get_visible_chunks);
    ClassDB::bind_method(D_METHOD("get_visible_entities", "chunk_coords", "cull_min", "cull_max", "max_entities"),&World::get_visible_entities);
    ClassDB::bind_method(D_METHOD("create_entity","type", "tile_coord", "entity_type", "entity_sprite"),&World::create_entity);
    ClassDB::bind_method(D_METHOD("get_entities_at_world_pos","coord"),&World::get_entities_at_world_pos);
    ClassDB::bind_method(D_METHOD("create_temp_job","jobPos", "entityPos", "id", "jobType"),&World::create_temp_job);

}

void World::init(int world_width_tiles, int world_height_tiles, int chunk_size_tiles) {
    if (chunk_size_tiles <= 0) chunk_size_tiles = 16;
    chunk_size = chunk_size_tiles;
    // UtilityFunctions::print("Chunk size in tiles: ", chunk_size);

    // Compute the number of chunks along each axis (ceiling)
    world_chunks_x = (world_width_tiles + chunk_size - 1) / chunk_size;
    world_chunks_y = (world_height_tiles + chunk_size - 1) / chunk_size;

    // No pre-generation: chunks load on-demand for massive worlds
    init_thread_pool(4);

    //Load JSON files for entity data
    // Preload the things we are most likely to use
    entityDataReader.preload_sheet(1);
    entityDataReader.preload_sheet(2);
}

// Convert world-space position (Vector3) to chunk coordinates.
// Uses X and Z axes (Y-up convention).
Vector2i World::world_pos_to_chunk(const Vector2 &pos) const {
    int tile_x = static_cast<int>(std::floor(pos.x));
    int tile_y = static_cast<int>(std::floor(pos.y)); // treat z as world Y for 2D grid
    return world_tile_to_chunk(tile_x, tile_y);
}

Vector2i World::world_pos_to_tile(const Vector2 &pos) const {
    int tile_x = static_cast<int>(std::floor(pos.x));
    int tile_y = static_cast<int>(std::floor(pos.y)); // treat z as world Y for 2D grid
    return Vector2i(tile_x, tile_y);

}

Vector2i World::world_tile_to_chunk(int tile_x,int tile_y) const {
    int cx = static_cast<int>(std::floor((double)tile_x / (double)chunk_size));
    int cy = static_cast<int>(std::floor((double)tile_y / (double)chunk_size));
    return Vector2i(cx, cy);
}

// Convert world tile to local tile within chunk
Vector2i World::world_tile_to_local(int tile_x, int tile_y) const {
    int lx = tile_x % chunk_size;
    int ly = tile_y % chunk_size;
    if (lx < 0) lx += chunk_size;
    if (ly < 0) ly += chunk_size;
    return Vector2i(lx, ly);
}

bool World::is_valid_chunk(const Vector2i &coord) const {
    return coord.x >= 0 && coord.x < world_chunks_x &&
        coord.y >= 0 && coord.y < world_chunks_y;
}




std::shared_ptr<Chunk> World::get_chunk(const Vector2i &coord) {
    // UtilityFunctions::print("Getting chunk");
    if (!is_valid_chunk(coord))
    {
        //  UtilityFunctions::print("Not valid chunk");
        return nullptr;
    }
    auto it = chunks.find(coord);
    if (it != chunks.end()) return it->second;

    //  UtilityFunctions::print("Returning nullptr");
    return nullptr;
}

std::shared_ptr<Chunk> World::load_chunk(const Vector2i &coord) {
    std::lock_guard<std::mutex> lock(chunks_mutex);

    // 1. Already loaded → return it
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        return it->second;
    }

    // 2. In backup → restore it
    auto backup_it = unloaded_chunk_backup.find(coord);
    if (backup_it != unloaded_chunk_backup.end()) {
        auto chunk = std::move(backup_it->second);
        unloaded_chunk_backup.erase(backup_it);

        chunks[coord] = chunk;
        //  UtilityFunctions::print("Restored chunk from backup: ", coord, 
        //     " (entities: ", chunk->entities.size(), ")");
        return chunk;
    }

    // 3. Brand new chunk → generate
    auto new_chunk = std::make_shared<Chunk>(chunk_size, chunk_size, coord, this);
    new_chunk->generate(coord.x * chunk_size, coord.y * chunk_size);  // assuming your generate signature
    chunks[coord] = new_chunk;

    // UtilityFunctions::print("Generated new chunk: ", coord);
    return new_chunk;
}

// world.cpp
void World::unload_chunk(const Vector2i &coord) {
    std::lock_guard<std::mutex> lock(chunks_mutex);

    auto it = chunks.find(coord);
    if (it == chunks.end()) return;

    // Move to backup instead of destroying
    unloaded_chunk_backup[coord] = std::move(it->second);
    chunks.erase(it);

    //  UtilityFunctions::print("Backed up chunk ", coord, " (entities: ", 
    //     unloaded_chunk_backup[coord]->entities.size(), ")");
}

int World::get_chunk_entity_count(const Vector2i &coord) const {
    auto it = chunks.find(coord);
    if (it == chunks.end()) return 0;
    return static_cast<int>(it->second->entities.size());
}

Vector2i World::get_entity_position(const Vector2i &chunk_coord, int entity_index) const {
    auto it = chunks.find(chunk_coord);
    if (it == chunks.end() || entity_index < 0 || entity_index >= it->second->entities.size()) {
        return Vector2(-99999, -99999); // invisible
    }
    return it->second->entities[entity_index]->get_position();  // already world space!
}

int World::get_chunk_entity_capacity() const {
    return chunk_size * chunk_size; // or make it a constant
}

int World::get_tile(int world_x, int world_y) const {
    Vector2i chunk_coord = world_tile_to_chunk(world_x, world_y);
    if (!is_valid_chunk(chunk_coord)) return 0;
    auto it = chunks.find(chunk_coord);
    if (it == chunks.end()) return 0;
    Vector2i local = world_tile_to_local(world_x, world_y);
    return it->second->get_tile(local.x, local.y);
}

int World::get_chunk_width() const{
    return world_chunks_x;
}

int World::get_chunk_height() const{
    return world_chunks_y;
}

int World::get_world_width_tiles() const {
    return world_chunks_x * chunk_size;
}

int World::get_world_height_tiles() const {
    return world_chunks_y * chunk_size;
}

int World::get_chunk_size() const {
    return chunk_size;
}

void World::set_tile(int world_x, int world_y, int value) {
    Vector2i chunk_coord = world_tile_to_chunk(world_x, world_y);
    if (!is_valid_chunk(chunk_coord)) return;
    auto it = chunks.find(chunk_coord);
    if (it == chunks.end()) {
        auto new_chunk = std::make_shared<Chunk>(chunk_size, chunk_size, chunk_coord,this);
        new_chunk->generate(world_x, world_y);
        chunks.emplace(chunk_coord, new_chunk);
        it = chunks.find(chunk_coord);
    }
    Vector2i local = world_tile_to_local(world_x, world_y);
    it->second->set_tile(local.x, local.y, value);
}

Array World::get_chunk_colors(const Vector2i &coord) {
    auto chunk = get_chunk(coord);
    if (!chunk) {
        return Array();
    }

    const std::vector<Color>& src = chunk->get_tile_colors();
    Array dest;
    dest.resize((int)src.size());

    for (size_t i = 0; i < src.size(); ++i) {
        dest[i] = src[i];   // Color → Variant conversion works automatically
    }
    return dest;
}




TypedArray<Vector2i> World::get_visible_chunks(
        const Vector2 &cam_pos,
        const Vector2 &world_min,
        const Vector2 &world_max,
        int max_render_distance
        ) {
    TypedArray<Vector2i> result;

    Vector2i min_chunk = world_pos_to_chunk(world_min);
    Vector2i max_chunk = world_pos_to_chunk(world_max);
    Vector2i origin_chunk = world_pos_to_chunk(cam_pos);

    // Clamp to max render distance
    Vector2i clamped_min(
            std::max(origin_chunk.x - max_render_distance, min_chunk.x),
            std::max(origin_chunk.y - max_render_distance, min_chunk.y)
            );
    Vector2i clamped_max(
            std::min(origin_chunk.x + max_render_distance, max_chunk.x),
            std::min(origin_chunk.y + max_render_distance, max_chunk.y)
            );

    float chunk_size_f = static_cast<float>(chunk_size);

    // Load and cull chunks in one pass
    for (int cy = clamped_min.y; cy <= clamped_max.y; ++cy) {
        for (int cx = clamped_min.x; cx <= clamped_max.x; ++cx) {
            Vector2i c(cx, cy);
            if (!is_valid_chunk(c)) continue;

            // Chunk-level AABB culling
            float chunk_world_min_x = c.x * chunk_size_f;
            float chunk_world_min_y = c.y * chunk_size_f;
            float chunk_world_max_x = (c.x + 1) * chunk_size_f;
            float chunk_world_max_y = (c.y + 1) * chunk_size_f;

            // Skip if chunk doesn't overlap visible area
            if (chunk_world_max_x < world_min.x || chunk_world_min_x > world_max.x ||
                    chunk_world_max_y < world_min.y || chunk_world_min_y > world_max.y) {
                continue;
            }

            // Load chunk if needed
            auto chunk = get_chunk(c);
            if (!chunk) chunk = load_chunk(c);

            if (chunk) {
                result.push_back(c);
            }
        }
    }

    return result;
}
Dictionary World::get_entities_at_world_pos(const Vector2 coord) {  
    Dictionary result;
    Vector2i entity_coord = world_pos_to_tile(coord);  // Assuming this returns Vector2i

    Vector2i chunk_coord = world_pos_to_chunk(entity_coord);
    auto chunk = load_chunk(chunk_coord);
    if (chunk == nullptr) {
        result["count"] = 0;
        return result;
    }

    int max_entities = chunk_size;  // Arbitrary cap; adjust as needed
    PackedInt64Array entity_ids;
    PackedInt32Array types;
    PackedInt32Array entity_sprites;
    PackedInt32Array entity_widths;  
    PackedInt32Array entity_heights;
    PackedInt32Array x_pos; 
    PackedInt32Array y_pos;

    // Pre-allocate for performance
    entity_ids.resize(max_entities);
    types.resize(max_entities);
    entity_sprites.resize(max_entities);
    entity_widths.resize(max_entities);
    entity_heights.resize(max_entities);
    x_pos.resize(max_entities);
    y_pos.resize(max_entities);

    int count = 0;
    for (const auto& entity_ptr : chunk->entities) {  // const ref for safety
        if (!entity_ptr) continue;
        Vector2 entity_pos = entity_ptr->get_position();
        // Exact match; for float tolerance: if (abs(entity_pos.x - entity_coord.x) < 0.01f && same for y)
        if (Math::absf(entity_pos.x - entity_coord.x) < 0.01f && Math::absf(entity_pos.y - entity_coord.y) < 0.01f) {
            if (count >= max_entities) {
                break;
            }
            entity_ids[count] = static_cast<int64_t>(entity_ptr->get_entity_id());  // Fixed: [] instead of .set
            types[count] = entity_ptr->get_type_id();
            entity_sprites[count] = entity_ptr->get_entity_sprite();
            entity_widths[count] = entity_ptr->get_entity_width();
            entity_heights[count] = entity_ptr->get_entity_height();
            x_pos[count] = entity_ptr->get_position().x;
            y_pos[count] = entity_ptr->get_position().y;
            count++;
        }
    }
    {
        std::lock_guard<std::mutex> lock(pending_mutex);
        for (const auto& [pending_chunk, entity_ptr] : pendingEntityPlacements) {
            if (!entity_ptr) continue;

            // Check if this pending entity is for our target chunk
            if (pending_chunk->coord != chunk_coord) continue;

            Vector2 entity_pos = entity_ptr->get_position();

            if (Math::absf(entity_pos.x - entity_coord.x) < 0.01f && 
                    Math::absf(entity_pos.y - entity_coord.y) < 0.01f) {

                if (count >= max_entities) break;

                entity_ids[count] = static_cast<int64_t>(entity_ptr->get_entity_id());
                types[count] = entity_ptr->get_type_id();
                entity_sprites[count] = entity_ptr->get_entity_sprite();
                entity_widths[count] = entity_ptr->get_entity_width();
                entity_heights[count] = entity_ptr->get_entity_height();
                x_pos[count] = entity_ptr->get_position().x;
                y_pos[count] = entity_ptr->get_position().y;
                count++;
            }
        }
    }

    entity_ids.resize(count);
    types.resize(count);
    entity_sprites.resize(count);
    entity_widths.resize(count);
    entity_heights.resize(count);
    x_pos.resize(count);
    y_pos.resize(count);

    result["entity_ids"] = entity_ids;
    result["types"] = types;
    result["entity_sprites"] = entity_sprites;
    result["entity_width"] = entity_widths; 
    result["entity_height"] = entity_heights;
    result["x_pos"] = x_pos;
    result["y_pos"] = y_pos;
    result["count"] = count;
    return result;
}

Dictionary World::get_visible_entities(
        const TypedArray<Vector2i>& chunk_coords,
        const Vector2& cull_min,
        const Vector2& cull_max,
        int max_entities)
{
    PackedVector2Array positions;
    PackedInt64Array entity_ids;
    PackedInt32Array types;
    PackedInt32Array entity_sprites;
    PackedInt32Array entity_width;
    PackedInt32Array entity_height;

    // Pre-allocate for performance
    positions.resize(max_entities);
    entity_ids.resize(max_entities);
    types.resize(max_entities);
    entity_sprites.resize(max_entities);
    entity_width.resize(max_entities);
    entity_height.resize(max_entities);

    int count = 0;

    for (int i = 0; i < chunk_coords.size() && count < max_entities; ++i) {
        Vector2i coord = chunk_coords[i];
        auto it = chunks.find(coord);
        if (it == chunks.end()) continue;

        const auto& chunk_entities = it->second->entities;
        for (const auto& entity_ptr : chunk_entities) {
            if (count >= max_entities) break;
            if (!entity_ptr || !entity_ptr->is_active()) continue;

            Vector2 pos = entity_ptr->get_position();  // Assuming get_position() returns Vector2 now

            // Entity-level culling
            if (pos.x < cull_min.x || pos.x > cull_max.x ||
                    pos.y < cull_min.y || pos.y > cull_max.y) {
                continue;
            }

            positions.set(count, pos);
            entity_ids.set(count, static_cast<int64_t>(entity_ptr->get_entity_id()));
            types.set(count, entity_ptr->get_type_id());
            entity_sprites.set(count, entity_ptr->get_entity_sprite());
            entity_width.set(count, entity_ptr->get_entity_width());
            entity_height.set(count, entity_ptr->get_entity_height());

            count++;
        }
    }

    // Trim arrays to actual used size (important for GDScript)
    positions.resize(count);
    entity_ids.resize(count);
    types.resize(count);
    entity_sprites.resize(count);

    // Build and return the dictionary
    Dictionary result;
    result["positions"]  = positions;
    result["entity_ids"] = entity_ids;
    result["types"]      = types;
    result["entity_sprites"] = entity_sprites;
    result["entity_width"] = entity_width;
    result["entity_height"] = entity_height;
    result["count"]      = count;

    return result;
}
void World::init_thread_pool(size_t thread_count) {
    if (thread_count == 0) {
        thread_count = std::max(2u, std::thread::hardware_concurrency() - 1);
    }
    thread_pool = std::make_unique<ThreadPool>(thread_count);
}


void World::request_chunk(Vector2i c) {
    if (!is_valid_chunk(c)) return;

    if (chunks.find(c) != chunks.end()) return;
    if (queued_chunks.find(c) != queued_chunks.end()) return;

    queued_chunks.insert(c);
    load_queue.push(c);
}

void World::process_chunk_loading() {
    constexpr int MAX_PER_FRAME = 2;

    std::lock_guard lock(chunks_mutex);

    int count = 0;
    while (!load_queue.empty() && count++ < MAX_PER_FRAME) {
        Vector2i c = load_queue.front();
        load_queue.pop();
        queued_chunks.erase(c);

        auto chunk = std::make_shared<Chunk>(chunk_size, chunk_size, c, this);
        chunk->generate(c.x * chunk_size, c.y * chunk_size);
        chunks[c] = chunk;
    }
}

void World::update(const Vector2 &origin,
                   int max_render_distance_chunks,
                   int simulation_distance,
                   float delta,
                   bool paused)
{
    if (paused) return;

    if (!thread_pool) init_thread_pool();

    // 1. Flush pending placements first (important!)
    {
        std::lock_guard lk(pending_mutex);
        if (!pendingEntityPlacements.empty()) {
            std::lock_guard lk2(chunks_mutex);
            for (auto& [target_chunk, e] : pendingEntityPlacements) {
                target_chunk->entities.push_back(e);
            }
            pendingEntityPlacements.clear();
        }
    }

    Vector2i origin_chunk_coord = world_pos_to_chunk(origin);
    int render_R = max_render_distance_chunks;
    int sim_R = simulation_distance;

    sim_cache.clear();

    //1: Decide which chunks we want this frame
    {
        std::lock_guard lk(chunks_mutex);

        // A. Always fully load + full-sim everything in render radius
        for (int dy = -render_R; dy <= render_R; ++dy) {
            for (int dx = -render_R; dx <= render_R; ++dx) {
                Vector2i c = origin_chunk_coord + Vector2i(dx, dy);
                if (!is_valid_chunk(c)) continue;

                sim_cache.needed.insert(c);

                auto it = chunks.find(c);
                std::shared_ptr<Chunk> ch;
                if (it == chunks.end()) {
                    request_chunk(c);           // will be generated soon
                    continue;
                }
                ch = it->second;

                if (!ch->entities.empty()) {
                    sim_cache.full_sim.push_back(ch);
                }
            }
        }

        // B. Light-sim ONLY already loaded chunks in simulation ring
        //    (never generate new distant chunks automatically)
        for (int dy = -sim_R; dy <= sim_R; ++dy) {
            for (int dx = -sim_R; dx <= sim_R; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) <= render_R) continue; // already handled

                Vector2i c = origin_chunk_coord + Vector2i(dx, dy);
                if (!is_valid_chunk(c)) continue;

                sim_cache.needed.insert(c);   // prevent unloading

                auto it = chunks.find(c);
                if (it == chunks.end()) continue; // do NOT generate distant chunks

                auto& ch = it->second;
                if (!ch->entities.empty()) {
                    sim_cache.light_sim.push_back(ch);
                }
            }
        }
    }

    // 2: Simulate (parallel where possible)
    if (!sim_cache.full_sim.empty()) {
        std::vector<std::future<void>> fs;
        fs.reserve(sim_cache.full_sim.size());
        for (auto& ch : sim_cache.full_sim) {
            fs.push_back(thread_pool->enqueue([ch, delta] {
                ch->simulate(delta, true);
            }));
        }
        for (auto& f : fs) f.get();
    }

    if (!sim_cache.light_sim.empty()) {
        if (sim_cache.light_sim.size() >= 8) {
            std::vector<std::future<void>> fs;
            fs.reserve(sim_cache.light_sim.size());
            for (auto& ch : sim_cache.light_sim) {
                fs.push_back(thread_pool->enqueue([ch, delta] {
                    ch->simulate(delta, false);
                }));
            }
            for (auto& f : fs) f.get();
        } else {
            for (auto& ch : sim_cache.light_sim) {
                ch->simulate(delta, false);
            }
        }
    }

    // Flush any new pending placements created during simulation
    {
        std::lock_guard lk(pending_mutex);
        if (!pendingEntityPlacements.empty()) {
            std::lock_guard lk2(chunks_mutex);
            for (auto& [tgt, e] : pendingEntityPlacements) {
                tgt->entities.push_back(e);
            }
            pendingEntityPlacements.clear();
        }
    }

    // 3: Unload anything we no longer need
    {
        std::lock_guard lk(chunks_mutex);
        std::vector<Vector2i> to_unload;
        for (const auto& [pos, _] : chunks) {
            if (sim_cache.needed.find(pos) == sim_cache.needed.end()) {
                to_unload.push_back(pos);
            }
        }
        for (auto c : to_unload) {
            unload_chunk(c);
        }
    }

    //4: Gradually generate requested chunks
    process_chunk_loading();  

    // Debug print (keep for now)
    UtilityFunctions::print("Update end | loaded=", chunks.size(),
                            " | full_sim=", sim_cache.full_sim.size(),
                            " | light_sim=", sim_cache.light_sim.size(),
                            " | queue size=", load_queue.size());
}
void World::create_entity(const String &type, const Vector2i &tile_coord, const int &entity_type, const int &entity_sprite)
{
    Vector2i chunk_coord = world_tile_to_chunk(tile_coord.x, tile_coord.y);
    // UtilityFunctions::print("-> Chunk: ", chunk_coord);
    //   Vector2i local = world_tile_to_local(tile_coord.x, tile_coord.y);
    // UtilityFunctions::print("-> Local: ", local);

    // Load the chunk if needed
    auto chunk = load_chunk(chunk_coord);
    if (!chunk) {
        UtilityFunctions::push_warning("Failed to load chunk for placement at ", tile_coord);
        return;
    }

    EntityData data = entityDataReader.get_entity_data(entity_type, entity_sprite);
    if(data.def) return;

    Vector2i size = Vector2i(data.size_x, data.size_y);

    if(type == "colonist")
    {

        auto e = std::make_shared<Colonist>(tile_coord, get_next_entity_id(), entity_sprite, size);
        e->set_move_speed(data.base_move_speed);
        pendingEntityPlacements.push_back({chunk, e});
    }
    else if(type == "building")
    {
        // temporarily make a building this size
        auto e = std::make_shared<Building>(tile_coord, get_next_entity_id(), entity_sprite, size, entity_type);
        e->set_move_speed(data.base_move_speed);
        pendingEntityPlacements.push_back({chunk,e});
        // How will i fetch the data for the buildings?
        // Storage space, size, available jobs, etc. 
        // Maybe just a simple data structure that is populated from a JSON or something
    }
    else if(type == "item")
    {
        // temporarily make a building this size
        auto e = std::make_shared<Item>(tile_coord, get_next_entity_id(), entity_sprite, size);
        e->set_move_speed(data.base_move_speed);
        pendingEntityPlacements.push_back({chunk,e});
    }
    else 
    {

        // auto e = std::make_shared<Building>(tile_coord, get_next_entity_id(), entity_type);
        //   e->set_type(entity_type);
        //     pendingEntityPlacements.push_back({chunk,e});

    }
    UtilityFunctions::print("Created ", type, " at (", tile_coord.x , ", ", tile_coord.y, ") with size (", size.x, ", ", size.y, ")");



}

// I think jobs should be made through a factory. That way certain entities can only have certain jobs and the UI will be easier to maintain
// But just a "template" system could work the same. UI could just have hard coded stuff anyways
void World::create_temp_job(const Vector2i& jobPos, const Vector2i& entityPos, const int& id, const int& jobType)
{
    Vector2i primary_chunk = world_tile_to_chunk(entityPos.x, entityPos.y);

    // Search this chunk and all 8 neighbors (same as get_entities_at_world_pos)
    std::vector<Vector2i> chunks_to_check;
    chunks_to_check.reserve(9);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            Vector2i check_chunk(primary_chunk.x + dx, primary_chunk.y + dy);
            if (is_valid_chunk(check_chunk)) {
                chunks_to_check.push_back(check_chunk);
            }
        }
    }

    // Lock to safely read chunks
    std::lock_guard<std::mutex> lock(chunks_mutex);

    // Search all relevant chunks for the entity
    for (const auto& chunk_coord : chunks_to_check) {
        auto chunk = get_chunk(chunk_coord);
        if (!chunk) continue;

        for (const auto& e : chunk->entities) {
            if (!e) continue;


            if (e->get_entity_id() == id) {
                int e_type = e->get_type_id();
                EntityJob job = create_job(e_type, jobType, jobPos);
                e->add_job(job);

                //    UtilityFunctions::print("Created job to go from ", entityPos, " to ", jobPos, 
                //      " (found in chunk ", chunk_coord, ") with job type ", jobType);
                return;
            }
        }
    }

    // Also check pending entities
    {
        std::lock_guard<std::mutex> pending_lock(pending_mutex);
        for (const auto& [pending_chunk, e] : pendingEntityPlacements) {
            if (!e) continue;

            if (e->get_entity_id() == id) {
                int e_type = e->get_type_id();
                EntityJob job = create_job(e_type, jobType, jobPos);
                e->add_job(job);

                // UtilityFunctions::print("Created job to go from ", entityPos, " to ", jobPos, 
                //     " (found in chunk ", pending_chunk->coord, ") with job type ", jobType);
                return;

            }
        }
    }

    UtilityFunctions::push_warning("Failed to find entity at ", entityPos, 
            " (searched chunk ", primary_chunk, " and neighbors)");
}

EntityJob getEntityJobConfig(const int& entityType, const int& jobType)
{   
    EntityJob config;

    if (entityType == 1 && jobType == 1) { // colonist speedy move to point
        config.move_algo = "default";
        config.moveSpeedMultiplier = 5.0f;
        config.priority = 100;
    } else if (entityType == 1 && jobType == 2) { //colonist slow move to point
        config.move_algo = "default";
        config.moveSpeedMultiplier = 1.0f;
        config.priority = 100;
    }

    return config;
}
EntityJob World::create_job(const int& entityType, const int& jobType, const Vector2i& jobPos)
{

    EntityJob job = getEntityJobConfig(entityType, jobType);
    job.target_coord = {jobPos.x,jobPos.y};

    return job;
}

