#include "world.h"
#include "chunk.h"
#include "variant/vector2i.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>
#include <unordered_set>

using namespace godot;


void World::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "world_width_tiles", "world_height_tiles", "chunk_size_tiles"),
                         &World::init);
    ClassDB::bind_method(D_METHOD("update", "origin", "render_distance_chunks", "delta"),
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

}

void World::init(int world_width_tiles, int world_height_tiles, int chunk_size_tiles) {
    if (chunk_size_tiles <= 0) chunk_size_tiles = 16;
    chunk_size = chunk_size_tiles;

    // Compute the number of chunks along each axis (ceiling)
    world_chunks_x = (world_width_tiles + chunk_size - 1) / chunk_size;
    world_chunks_y = (world_height_tiles + chunk_size - 1) / chunk_size;

    // No pre-generation: chunks load on-demand for massive worlds
}

// Convert world-space position (Vector3) to chunk coordinates.
// Uses X and Z axes (Y-up convention).
Vector2i World::world_pos_to_chunk(const Vector2 &pos) const {
    int tile_x = static_cast<int>(std::floor(pos.x));
    int tile_y = static_cast<int>(std::floor(pos.y)); // treat z as world Y for 2D grid
    return world_tile_to_chunk(tile_x, tile_y);
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
    if (!is_valid_chunk(coord)) return nullptr;
    auto it = chunks.find(coord);
    if (it != chunks.end()) return it->second;
    return nullptr;
}

std::shared_ptr<Chunk> World::load_chunk(const Vector2i &coord) {
    if (!is_valid_chunk(coord)) return nullptr;
    auto existing = get_chunk(coord);
    if (existing) return existing;

    auto c = std::make_shared<Chunk>(chunk_size, chunk_size, coord, this);
    c->generate(1,1);
    //c->load();
    chunks.emplace(coord, c);
    return c;
}

void World::unload_chunk(const Vector2i &coord) {
    chunks.erase(coord);
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

void World::update(const Vector2 &origin, int render_distance_chunks, float delta) {


    for(auto ec : pendingEntityPlacements)
    {
        auto chunkRef = std::get<0>(ec);
        auto entityRef = std::get<1>(ec);

        chunkRef->entities.push_back(entityRef);
    }
    pendingEntityPlacements.clear();

    Vector2i origin_chunk = world_pos_to_chunk(origin);
    int R = render_distance_chunks;
    std::unordered_set<Vector2i, Vector2iHash> needed_chunks;

    // Load and simulate nearby chunks
    for (int dy = -R; dy <= R; ++dy) {
        for (int dx = -R; dx <= R; ++dx) {
            Vector2i c = origin_chunk + Vector2i(dx, dy);
            if (!is_valid_chunk(c)) continue;
            needed_chunks.insert(c);

            auto chunk = get_chunk(c);
            if (!chunk) chunk = load_chunk(c);
            if (chunk) chunk->simulate(delta, true);
        }
    }

    // Light simulation for loaded chunks outside render distance
    for (auto &kv : chunks) {
        const Vector2i &coord = kv.first;
        auto &chunk = kv.second;
        int dist_x = std::abs(coord.x - origin_chunk.x);
        int dist_y = std::abs(coord.y - origin_chunk.y);
        if (dist_x > R || dist_y > R) {
            chunk->simulate(delta, false);
        }
    }

    // Unload far chunks
    std::vector<Vector2i> to_remove;
    for (auto &kv : chunks) {
        if (needed_chunks.find(kv.first) == needed_chunks.end()) {
            to_remove.push_back(kv.first);
        }
    }
    for (auto &c : to_remove) unload_chunk(c);
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

    if(type == "colonist")
    {

        auto e = std::make_shared<Colonist>(tile_coord, get_next_entity_id(), entity_sprite, Vector2i(1,1));
        pendingEntityPlacements.push_back({chunk,e});
    }
    else if(type == "building")
    {
        // temporarily make a building this size
        auto e = std::make_shared<Building>(tile_coord, get_next_entity_id(), entity_sprite, Vector2i(1,1), entity_type);
        pendingEntityPlacements.push_back({chunk,e});
    }
    else if(type == "item")
    {
        // temporarily make a building this size
        auto e = std::make_shared<Item>(tile_coord, get_next_entity_id(), entity_sprite, Vector2i(1,1));
        pendingEntityPlacements.push_back({chunk,e});
    }
    else 
    {

       // auto e = std::make_shared<Building>(tile_coord, get_next_entity_id(), entity_type);
     //   e->set_type(entity_type);
   //     pendingEntityPlacements.push_back({chunk,e});
    
    }



}
