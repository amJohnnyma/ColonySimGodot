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

            if (n < bd.temp_entity_spawn_rate) {
                Vector2 world_pos(
                    coord.x * width + x + 0.5f,
                    coord.y * height + y + 0.5f
                );
                auto e = std::make_shared<Colonist>(world_pos, world->get_next_entity_id());

                thread_local static std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<int> dist(0, 5);

                e->set_type(dist(gen));

                entities.push_back(e);
            }
            
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
    std::vector<std::shared_ptr<Entity>> to_transfer[4]; // N=0, E=1, S=2, W=3

    //UtilityFunctions::print("Chunk(",coord.x, coord.y,") has ", entities.size(), "entities before");
    
    for (int i = 0; i < (int)entities.size(); ++i) {
        if (!entities[i]) continue;
     //   std::vector<std::tuple<Vector2i, int>> neighbourChunksCoords = getNeighbouringChunks(entities[i]->position, world, coord);
        std::vector<int> availableDirs = {}; // getAvailableDirs(entityWorldToLocalCoord(entities[i]->position, world), neighbourChunksCoords);
        Vector2i new_pos;
        EntitySimulationParam params = 
        {
            delta,
            new_pos,
            availableDirs
        };
        bool moved = entities[i]->simulate(params);
        
        if (moved && full_simulation) {
            // make sure they stay in world bounds
            if(new_pos.x < 0 || new_pos.x >= world->get_world_width_tiles())  continue;
            if(new_pos.y < 0 || new_pos.y >= world->get_world_height_tiles()) continue;

            entities[i]->set_position(new_pos);
           // UtilityFunctions::print("Entity moved from ", old_pos, "to ", new_pos);
            //Vector2i local_pos(new_pos.x - chunk_origin.x, new_pos.y - chunk_origin.y);
            Vector2i local_pos(new_pos % world->get_chunk_size());
            
            // Check if entity left this chunk's bounds
            if (local_pos.x < 0) {
                to_transfer[3].push_back(entities[i]);
             //   UtilityFunctions::print("Transferring WEST");
                entities[i] = nullptr;
            }
            else if (local_pos.x >= width) {
                to_transfer[1].push_back(entities[i]);
              //  UtilityFunctions::print("Transferring EAST");
                entities[i] = nullptr;
            }
            else if (local_pos.y < 0) {
                to_transfer[0].push_back(entities[i]);
                //UtilityFunctions::print("Transferring NORTH");
                entities[i] = nullptr;
            }
            else if (local_pos.y >= height) {
                to_transfer[2].push_back(entities[i]);
                //UtilityFunctions::print("Transferring SOUTH");
                entities[i] = nullptr;
            }
            // else: entity stayed in chunk, no transfer needed
        }
    }
    
    // Transfer entities to neighboring chunks
    if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1)); //n
    if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0)); //e
    if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1)); //s
    if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0)); //w


    entities.erase(
        std::remove(entities.begin(), entities.end(), nullptr),
        entities.end()
    );
   // UtilityFunctions::print("Chunk(",coord.x, coord.y,") has ", entities.size(), "entities after");
}

void Chunk::transfer_entities(std::vector<std::shared_ptr<Entity>>& entities_vec, Vector2i direction) {
    Vector2i target_chunk = coord + direction;
    auto target = world->get_chunk(target_chunk);
    if (!target) {
        target = world->load_chunk(target_chunk);
    }
    if (target) {
        for (auto& e : entities_vec) {
            // Position is already wrapped by the calling function
            target->entities.push_back(e);
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
