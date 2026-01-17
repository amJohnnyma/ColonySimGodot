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
    UtilityFunctions::print("Chunk size: ",width, " x ", height);

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

// Helper function to check if two entity rectangles overlap
bool entities_overlap(Vector2i pos1, Vector2i size1, Vector2i pos2, Vector2i size2)
{
    // Position is bottom-left corner
    // Entity at pos (2,2) with size (2,2) occupies: (2,2), (3,2), (2,1), (3,1)
    // X range: [pos.x, pos.x + size.x - 1] → [2, 3]
    // Y range: [pos.y - size.y + 1, pos.y] → [1, 2]
    
    int left1 = pos1.x;
    int right1 = pos1.x + size1.x - 1;
    int top1 = pos1.y - size1.y + 1;  // Y decreases going up
    int bottom1 = pos1.y;
    
    int left2 = pos2.x;
    int right2 = pos2.x + size2.x - 1;
    int top2 = pos2.y - size2.y + 1;
    int bottom2 = pos2.y;
    
    // Check for no overlap using AABB collision
    // No overlap if one rectangle is completely to the left, right, above, or below the other
    if (right1 < left2 || right2 < left1 ||
        bottom1 < top2 || bottom2 < top1)
    {
        return false; // No overlap
    }
    
    return true; // Overlap detected
}
std::vector<int> Chunk::getAvailableDirs(Vector2i current_world, Vector2i current_size,
                                          std::vector<std::tuple<Vector2i, int>> neighbourChunks)
{
    std::vector<int> blocked_dirs;
    
    if (!world) {
        UtilityFunctions::print("ERROR: Chunk::world is null!");
        return {0, 1, 2, 3}; // Return all dirs as available if no world
    }

    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)
    };
    
    UtilityFunctions::print("Checking collision for entity at world pos: ", current_world.x, ",", current_world.y, 
                           " size: ", current_size.x, "x", current_size.y);
    
    // Check current chunk entities
    UtilityFunctions::print("  Current chunk has ", entities.size(), " entities");
    for (size_t i = 0; i < entities.size(); i++)
    {
        const auto& e = entities[i];
        if (!e) {
            UtilityFunctions::print("  Entity ", i, " is null, skipping");
            continue;
        }
        
        Vector2i eWorld = e->get_position();
        Vector2i eSize = e->get_entity_size();
        
        UtilityFunctions::print("  Checking against entity at: ", eWorld.x, ",", eWorld.y, 
                               " size: ", eSize.x, "x", eSize.y);
        
        // Skip self-collision (same position)
        if (eWorld == current_world) {
            UtilityFunctions::print("    Same position, skipping self");
            continue;
        }
        
        // Check each direction
        for (int dir = 0; dir < 4; dir++)
        {
            Vector2i test_pos = current_world + dirs[dir];
            
            // Check if moving in this direction would cause overlap
            if (entities_overlap(test_pos, current_size, eWorld, eSize))
            {
                UtilityFunctions::print("    Direction ", dir, " blocked by this entity (test_pos: ", 
                                       test_pos.x, ",", test_pos.y, ")");
                blocked_dirs.push_back(dir);
            }
        }
    }
    
    // Check neighbour chunk entities
    UtilityFunctions::print("  Checking ", neighbourChunks.size(), " neighbour chunks");
    for (const auto& [chunk_coord, dir] : neighbourChunks)
    {
        UtilityFunctions::print("  Neighbour chunk at: ", chunk_coord.x, ",", chunk_coord.y, " dir: ", dir);
        
        auto chunk = world->get_chunk(chunk_coord);
        if (!chunk) {
            UtilityFunctions::print("    Chunk is null, skipping");
            continue;
        }
        
        UtilityFunctions::print("    Neighbour has ", chunk->entities.size(), " entities");
        
        for (size_t i = 0; i < chunk->entities.size(); i++)
        {
            const auto& e = chunk->entities[i];
            if (!e) {
                UtilityFunctions::print("    Entity ", i, " is null, skipping");
                continue;
            }
            
            Vector2i eWorld = e->get_position();
            Vector2i eSize = e->get_entity_size();
            
            // Skip self
            if (eWorld == current_world) {
                UtilityFunctions::print("    Same position as current entity, skipping");
                continue;
            }
            
            // Check the direction toward this neighbour chunk
            Vector2i test_pos = current_world + dirs[dir];
            
            if (entities_overlap(test_pos, current_size, eWorld, eSize))
            {
                UtilityFunctions::print("    Direction ", dir, " blocked by neighbour entity");
                blocked_dirs.push_back(dir);
            }
        }
    }
    
    // Remove duplicates
    std::sort(blocked_dirs.begin(), blocked_dirs.end());
    blocked_dirs.erase(std::unique(blocked_dirs.begin(), blocked_dirs.end()), blocked_dirs.end());
    
    // Return available directions (all except blocked)
    std::vector<int> available;
    for (int i = 0; i < 4; i++)
    {
        if (std::find(blocked_dirs.begin(), blocked_dirs.end(), i) == blocked_dirs.end())
        {
            available.push_back(i);
        }
    }
    
    UtilityFunctions::print("  Available dirs: ", available.size(), " out of 4");
    
    return available;
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
    if (entities.empty()) return;

    constexpr int MAX_STACK_TRANSFERS = 64;
    std::vector<std::shared_ptr<Entity>> to_transfer[4];

    if (entities.size() > MAX_STACK_TRANSFERS) {
        for (int i = 0; i < 4; ++i) {
            to_transfer[i].reserve(entities.size() / 16);
        }
    }

    const int chunk_size = world->get_chunk_size();
    const int world_width  = world->get_world_width_tiles();
    const int world_height = world->get_world_height_tiles();

    std::vector<std::shared_ptr<Entity>> staying_entities;
    staying_entities.reserve(entities.size());

    for (const auto& entity : entities) {
        if (!entity) continue;

        Vector2i new_pos;
        Vector2i entity_world_pos = entity->get_position();
        
        // Get neighbouring chunks if entity is on border
        auto neighbourChunks = getNeighbouringChunks(entity_world_pos, world, coord);
        
        // Get available directions based on collisions (using world coordinates)
        std::vector<int> availableDirs = getAvailableDirs(entity_world_pos, 
                                                           entity->get_entity_size(),
                                                           neighbourChunks);
        
        EntitySimulationParam params = {
            delta,
            new_pos,
            availableDirs
        };

        bool moved = entity->simulate(params);

        if (moved) {
            // Bounds check
            if (new_pos.x < 0 || new_pos.x >= world_width ||
                new_pos.y < 0 || new_pos.y >= world_height) {
                staying_entities.push_back(entity);
                continue;
            }

            entity->set_position(new_pos);
            Vector2i new_chunk = world->world_pos_to_chunk(new_pos);

            if (new_chunk == coord) {
                staying_entities.push_back(entity);
            }
            else {
                Vector2i direction = new_chunk - coord;
                
                if (direction.x != 0 && direction.y == 0) {
                    if (direction.x > 0) to_transfer[1].push_back(entity);
                    else to_transfer[3].push_back(entity);
                }
                else if (direction.y != 0 && direction.x == 0) {
                    if (direction.y > 0) to_transfer[2].push_back(entity);
                    else to_transfer[0].push_back(entity);
                }
                else {
                    if (std::abs(direction.x) > std::abs(direction.y)) {
                        if (direction.x > 0) to_transfer[1].push_back(entity);
                        else to_transfer[3].push_back(entity);
                    } else {
                        if (direction.y > 0) to_transfer[2].push_back(entity);
                        else to_transfer[0].push_back(entity);
                    }
                }
            }
        }
        else {
            staying_entities.push_back(entity);
        }
    }

    entities = std::move(staying_entities);

    if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1));
    if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0));
    if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1));
    if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0));
}
/*
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

    const int chunk_size = world->get_chunk_size();
    const int world_width  = world->get_world_width_tiles();
    const int world_height = world->get_world_height_tiles();

    // We'll collect entities that stay in this chunk
    std::vector<std::shared_ptr<Entity>> staying_entities;
    staying_entities.reserve(entities.size());

    // Simulate all entities
    for (const auto& entity : entities) {
        if (!entity) continue;

        Vector2i new_pos;
        EntitySimulationParam params = {
            delta,
            new_pos,
            {} // availableDirs empty since commented out
        };

        bool moved = entity->simulate(params);

        if (moved) {  // Removed && full_simulation — always apply movement if moved
            // Bounds check (world edges)
            if (new_pos.x < 0 || new_pos.x >= world_width ||
                new_pos.y < 0 || new_pos.y >= world_height) {
                // Entity would leave world bounds → don't move it
                staying_entities.push_back(entity);
                continue;
            }

            // Apply the new position
            entity->set_position(new_pos);
            // which chunk does this position belong to
            Vector2i new_chunk = world->world_pos_to_chunk(new_pos);

            if(new_chunk == coord) // is it still in this chunk?
            {
                staying_entities.push_back(entity);
            }
            else{
    // Crossed border → determine direction
                Vector2i direction = new_chunk - coord;
                
                // Clamp direction to one of the 4 cardinal directions
                if (direction.x != 0 && direction.y == 0) {
                    // East or West
                    if (direction.x > 0) to_transfer[1].push_back(entity); // East
                    else to_transfer[3].push_back(entity); // West
                }
                else if (direction.y != 0 && direction.x == 0) {
                    // North or South
                    if (direction.y > 0) to_transfer[2].push_back(entity); // South
                    else to_transfer[0].push_back(entity); // North
                }
                else {
                    // Diagonal or multi-chunk jump - just pick primary direction
                    if (std::abs(direction.x) > std::abs(direction.y)) {
                        if (direction.x > 0) to_transfer[1].push_back(entity);
                        else to_transfer[3].push_back(entity);
                    } else {
                        if (direction.y > 0) to_transfer[2].push_back(entity);
                        else to_transfer[0].push_back(entity);
                    }
                }
            }

        }
        else {
            // Didn't move → keep it
            staying_entities.push_back(entity);
        }
    }
        if (!to_transfer[0].empty() || !to_transfer[1].empty() || 
            !to_transfer[2].empty() || !to_transfer[3].empty()) {
            
            UtilityFunctions::print("Chunk ", coord.x, ",", coord.y, " transferring:");
            if (!to_transfer[0].empty()) UtilityFunctions::print("  North: ", to_transfer[0].size());
            if (!to_transfer[1].empty()) UtilityFunctions::print("  East: ", to_transfer[1].size());
            if (!to_transfer[2].empty()) UtilityFunctions::print("  South: ", to_transfer[2].size());
            if (!to_transfer[3].empty()) UtilityFunctions::print("  West: ", to_transfer[3].size());
        }

    // Replace the old list with only the entities that are still in this chunk
    entities = std::move(staying_entities);

    // Perform the actual transfers
    if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1));
    if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0));
    if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1));
    if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0));
}
*/
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
