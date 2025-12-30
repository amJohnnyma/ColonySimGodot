// chunk.cpp
#include "chunk.h"
#include <godot_cpp/variant/vector2i.hpp>
#include "variant/color.hpp"
#include "world.h"        
#include "entity.h"
#include "perlinNoise.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <memory>
#include <vector>

using namespace godot;


Chunk::Chunk(int w, int h, Vector2i c, World* parent_world)
    : world(parent_world), coord(c), width(w), height(h) {
    tiles.resize(width * height, 0);
    tileColors.resize(width * height, Color(1,1,1,1));
}

Chunk::Chunk(int w, int h, Vector2i c) : Chunk(w, h, c, nullptr) {}

void Chunk::generate(int wx, int wy) {
    PerlinNoise noise(12345);
    BiomeType b = noise.get_biome(wx, wy);
    const BiomeData bd = BIOME_TABLE[(int)b];

    entities.clear();
    entities.reserve(width * height / 50); // there wont be max entities in a chunk to start
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tiles[y * width + x] = (x + y + coord.x + coord.y) % 3;

            double n = noise.noise(
                double(x + coord.x * width) * 0.05,
                double(y + coord.y * height) * 0.05
            );
            float v = float((n + 1.0) * 0.5);
            tileColors[y * width + x] = Color(v * bd.color[0], v * bd.color[1], v * bd.color[2], bd.color[3]);
/*
            if (n < bd.temp_entity_spawn_rate) {
                Vector2 world_pos(
                    coord.x * width + x + 0.5f,
                    coord.y * height + y + 0.5f
                );
                auto e = std::make_shared<Colonist>(world_pos, world->get_next_entity_id(), 1);
                entities.push_back(e);
            }
  */
        }
    }

}

Vector2i entityWorldToLocalCoord(Vector2i worldCoord, World* world)
{
    int cs = world->get_chunk_size();
    // Proper positive modulo for negative numbers
    int x = worldCoord.x % cs;
    int y = worldCoord.y % cs;
    if (x < 0) x += cs;
    if (y < 0) y += cs;
    return Vector2i(x, y);
}
//entity pos in local coords
// neighbour chunks are only if applicable (On a chunk border)
std::vector<int> Chunk::getAvailableDirs(Vector2i current, std::vector<std::tuple<Vector2i, int>> neighbourChunks)
{
    std::vector<int> ret = {};
    if (!world) {
        UtilityFunctions::print("ERROR: Chunk::world is null!");
        return ret; // or {0,1,2,3} or whatever is safe
    }

    std::vector<std::shared_ptr<Chunk>> neighbour = {};
    int chunkSize = world->get_chunk_size();

    for(auto& coord : neighbourChunks)
    {
        // get the chunk coord
        // Check entities that will interfere with the direction
        // 0N -> Entity.pos.y = chunksize - 1 && Entity.pos.x == current.x -> Entity in other chunk on square we want
        // 1E -> Entity.pos.x = 0 && Entity.pos.y == current.y
        // 2S -> Entity.pos.y = 0 && Entity.pos.x == current.x
        // 3W -> Entity.pos.x = chunksize - 1 && Entity.pos.y == current.y
        
        auto chunk = world->get_chunk(std::get<0>(coord));
        auto dir = std::get<1>(coord);

        if (!chunk) {
            // Neighbour chunk not loaded — be safe: either skip or assume blocked
            // For now, let's assume direction is BLOCKED (don't add it)
            continue;
        }

        for(auto &e : chunk->entities)
        {
            if (!e) continue;
            Vector2i eLocal = entityWorldToLocalCoord(e->get_position(), world);
            switch(dir)
            {
                case 0:
                    if(eLocal.y == chunkSize -1 && eLocal.x != current.x)
                        ret.push_back(dir);
                    break;
                case 1:
                    if(eLocal.x == 0 && eLocal.y != current.y)
                        ret.push_back(dir);
                    break;
                case 2:
                    if(eLocal.y == 0 && eLocal.x != current.x)
                        ret.push_back(dir);
                    break;
                case 3:
                    if(eLocal.x == chunkSize -1 && eLocal.y != current.y)
                        ret.push_back(dir);
                    break;
                default:
                    break;
            }
        }
    }
    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)
    };
    // and check current chunk
    for(auto& e : entities)
    {
        if (!e) continue;
        Vector2i eLocal = entityWorldToLocalCoord(e->get_position(), world);
        for(int i = 0; i < 4; i ++)
        {
            if(current + dirs[i] != eLocal) ret.push_back(i);

        }
    }

    return ret;
}

// tuple int = dir -> 0,1,2,3 N E S W
std::vector<std::tuple<Vector2i, int>> getNeighbouringChunks(Vector2i entityWorldCoord, World* world, Vector2i chunkCoord)
{
    int chunkSize = world->get_chunk_size();
    int worldChunkWidth = world->get_chunk_width();
    int worldChunkHeight = world->get_chunk_height();

    std::vector<std::tuple<Vector2i, int>> ret = {};
    Vector2i entityLocalCoord = 
    {
        entityWorldCoord.x % chunkSize,

        entityWorldCoord.y % chunkSize
    };

    if(entityLocalCoord.x == 0) // we need the left chunk W
    {
        if(chunkCoord.x - 1 >= 0) // it can be added
               ret.push_back({Vector2i(chunkCoord.x - 1, chunkCoord.y), 3}); 
    }
    if(entityLocalCoord.x == chunkSize-1) // we need the right chunk E
    {
        if(chunkCoord.x + 1 < worldChunkWidth) // it can be added
               ret.push_back({Vector2i(chunkCoord.x + 1, chunkCoord.y), 1}); 
    }
    if(entityLocalCoord.y == 0) // we need the top chunk N
    {
        if(chunkCoord.y - 1 >= 0) // it can be added
               ret.push_back({Vector2i(chunkCoord.x, chunkCoord.y - 1), 0}); 
    }
    if(entityLocalCoord.y == chunkSize-1) // we need the bottom chunk S
    {
        if(chunkCoord.y + 1 < worldChunkHeight) // it can be added
               ret.push_back({Vector2i(chunkCoord.x, chunkCoord.y + 1), 2}); 
    }


    return ret;
}

void Chunk::simulate(float delta, bool full_simulation) {
    if (entities.empty()) return; // Early exit
    
    // Use stack allocation for small arrays
    constexpr int MAX_STACK_TRANSFERS = 64;
    std::vector<std::shared_ptr<Entity>> to_transfer[4];
    
    // Pre-allocate if we have many entities
    if (entities.size() > MAX_STACK_TRANSFERS) {
        for (int i = 0; i < 4; ++i) {
            to_transfer[i].reserve(entities.size() / 16);
        }
    }
    
    const int chunk_width = world->get_chunk_size();
    const int world_width = world->get_world_width_tiles();
    const int world_height = world->get_world_height_tiles();
    
    // Simulate all entities
    for (auto& entity : entities) {
        if (!entity) continue;
        
        Vector2i new_pos;
        EntitySimulationParam params = {
            delta,
            new_pos,
            {} // availableDirs empty since commented out
        };
        
        bool moved = entity->simulate(params);
        
        if (moved && full_simulation) {
            // Bounds check
            if (new_pos.x < 0 || new_pos.x >= world_width ||
                new_pos.y < 0 || new_pos.y >= world_height) {
                continue;
            }
            
            entity->set_position(new_pos);
            
            // Fast modulo for local position (works for positive numbers)
            int local_x = new_pos.x % chunk_width;
            int local_y = new_pos.y % chunk_width;
            
            // Check if entity left chunk bounds
            if (local_x < 0) {
                to_transfer[3].push_back(entity);
                entity = nullptr;
            }
            else if (local_x >= width) {
                to_transfer[1].push_back(entity);
                entity = nullptr;
            }
            else if (local_y < 0) {
                to_transfer[0].push_back(entity);
                entity = nullptr;
            }
            else if (local_y >= height) {
                to_transfer[2].push_back(entity);
                entity = nullptr;
            }
        }
    }
    
    // Batch transfers (reduce lock contention)
    if (to_transfer[0].size() + to_transfer[1].size() + 
        to_transfer[2].size() + to_transfer[3].size() > 0) {
        
        if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1));
        if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0));
        if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1));
        if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0));
    }
    
    // Single erase operation (more efficient than loop)
    if (!entities.empty()) {
        auto new_end = std::remove(entities.begin(), entities.end(), nullptr);
        if (new_end != entities.end()) {
            entities.erase(new_end, entities.end());
        }
    }
}

void Chunk::transfer_entities(
    std::vector<std::shared_ptr<Entity>>& entities_vec, 
    Vector2i direction) 
{
    if (entities_vec.empty() || !world) return;
    
    Vector2i target_chunk = coord + direction;
    std::shared_ptr<Chunk> target;
    
    {
        std::lock_guard<std::mutex> lock(world->chunks_mutex);
        target = world->get_chunk(target_chunk);
        if (!target) {
            target = world->load_chunk(target_chunk);
        }
    }
    
    if (target) {
        std::lock_guard<std::mutex> lock(world->pending_mutex);
        // Batch insert
        for (auto& e : entities_vec) {
            world->pendingEntityPlacements.emplace_back(target, e);
        }
    }
    
    entities_vec.clear();
}
int Chunk::get_tile(int local_x, int local_y) const {
    if (local_x < 0 || local_y < 0 || local_x >= width || local_y >= height) return 0;
    return tiles[local_y * width + local_x];
}

void Chunk::set_tile(int local_x, int local_y, int value) {
    if (local_x < 0 || local_y < 0 || local_x >= width || local_y >= height) return;
    tiles[local_y * width + local_x] = value;
}

std::vector<Color> Chunk::get_tile_colors() const {
    return tileColors;
}
