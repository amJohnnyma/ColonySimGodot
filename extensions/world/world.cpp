#include "world.h"
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

    auto c = std::make_shared<Chunk>(chunk_size, chunk_size, coord);
    c->generate(1,1);
    //c->load();
    chunks.emplace(coord, c);
    return c;
}

void World::unload_chunk(const Vector2i &coord) {
    chunks.erase(coord);
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
        auto new_chunk = std::make_shared<Chunk>(chunk_size, chunk_size, chunk_coord);
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

void World::update(const Vector2 &origin, int render_distance_chunks, float delta) {
    Vector2i origin_chunk = world_pos_to_chunk(origin);
    int R = render_distance_chunks;
    std::unordered_set<Vector2i, Vector2iHash> needed_chunks;

    // Load and full-simulate nearby chunks
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
