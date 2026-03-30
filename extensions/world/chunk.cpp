#include "chunk.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <algorithm>
#include <memory>
#include <vector>

#include "entity.h"
#include "terrainGenerator.h"
#include "world.h"

using namespace godot;

Chunk::Chunk(int w, int h, Vector2i c, World *parent_world)
    : world(parent_world), coord(c), width(w), height(h) {
    tiles.resize(width * height, 0);
    tileColors.resize(width * height, Color(1, 1, 1, 1));
}

Chunk::Chunk(int w, int h, Vector2i c) : Chunk(w, h, c, nullptr) {}

void Chunk::generate(int wx, int wy) {
    TerrainGenerator terrain(12345);
    entities.clear();
    entities.reserve(width * height / 50);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int world_x = x + coord.x * width;
            int world_y = y + coord.y * height;

            tiles[y * width + x] = (x + y + coord.x + coord.y) % 3;
            tileColors[y * width + x] = terrain.generate_tile_color(world_x, world_y);

        }
    }
    auto newentities = terrain.get_chunk_entities(width, coord);
    for(auto e : newentities)
    {
        // I know its just buildings rn
        Vector2i epos = std::get<1>(e);
        int ewidth = std::get<2>(e);
        int eheight = std::get<3>(e);

        if(!isOverlappingEntity(epos))
        {
          //  auto tree = std::make_shared<Building>(epos,world->get_next_entity_id(), 8, Vector2i(ewidth,eheight));
          //  entities.push_back(tree);
        }

    }
}

bool Chunk::isOverlappingEntity(Vector2i pos)
{
    for (const auto& e : entities)
    {
        Vector2i entity_pos  = e->get_position();   // bottom-left, integer
        Vector2i entity_size = e->get_entity_size();

        if (entity_size.x <= 0 || entity_size.y <= 0) continue;

        Vector2i entity_min = entity_pos;                               // bottom-left
        Vector2i entity_max = entity_pos + entity_size - Vector2i(1, 1); // inclusive top-right

        // pos is overlapped if it's inside [min .. max] inclusive
        if (pos.x >= entity_min.x &&
            pos.x <= entity_max.x &&
            pos.y >= entity_min.y - (entity_size.y - 1) &&  // = entity_pos.y - entity_size.y + 1
            pos.y <= entity_max.y)                          // = entity_pos.y
        {
            return true;  // yes — this cell is occupied
        }
    }
    return false;  // no overlap → free
}

Vector2i entityWorldToLocalCoord(Vector2i worldCoord, World *world) {
    int cs = world->get_chunk_size();
    int x = worldCoord.x % cs;
    int y = worldCoord.y % cs;
    if (x < 0) x += cs;
    if (y < 0) y += cs;
    return Vector2i(x, y);
}

bool entities_overlap(Vector2i pos1, Vector2i size1, Vector2i pos2, Vector2i size2) {
    int left1 = pos1.x;
    int right1 = pos1.x + size1.x - 1;
    int top1 = pos1.y - size1.y + 1;
    int bottom1 = pos1.y;

    int left2 = pos2.x;
    int right2 = pos2.x + size2.x - 1;
    int top2 = pos2.y - size2.y + 1;
    int bottom2 = pos2.y;

    if (right1 < left2 || right2 < left1 || bottom1 < top2 || bottom2 < top1) {
        return false;
    }

    return true;
}

std::vector<int> Chunk::getAvailableDirs(Vector2i current_world, Vector2i current_size,
                                         std::vector<std::tuple<Vector2i, int>> neighbourChunks) {
    std::vector<int> blocked_dirs;

    if (!world) {
        return {0, 1, 2, 3};
    }

    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)};

    for (size_t i = 0; i < entities.size(); i++) {
        const auto &e = entities[i];
        if (!e) {
            continue;
        }

        Vector2i eWorld = e->get_position();
        Vector2i eSize = e->get_entity_size();

        if (eWorld == current_world) {
            continue;
        }

        for (int dir = 0; dir < 4; dir++) {
            Vector2i test_pos = current_world + dirs[dir];

            if (entities_overlap(test_pos, current_size, eWorld, eSize)) {
                blocked_dirs.push_back(dir);
            }
        }
    }

    for (const auto &[chunk_coord, dir] : neighbourChunks) {
        auto chunk = world->get_chunk(chunk_coord);
        if (!chunk) {
            continue;
        }

        for (size_t i = 0; i < chunk->entities.size(); i++) {
            const auto &e = chunk->entities[i];
            if (!e) {
                continue;
            }

            Vector2i eWorld = e->get_position();
            Vector2i eSize = e->get_entity_size();

            if (eWorld == current_world) {
                continue;
            }

            Vector2i test_pos = current_world + dirs[dir];

            if (entities_overlap(test_pos, current_size, eWorld, eSize)) {
                blocked_dirs.push_back(dir);
            }
        }
    }

    std::sort(blocked_dirs.begin(), blocked_dirs.end());
    blocked_dirs.erase(std::unique(blocked_dirs.begin(), blocked_dirs.end()), blocked_dirs.end());

    std::vector<int> available;
    for (int i = 0; i < 4; i++) {
        if (std::find(blocked_dirs.begin(), blocked_dirs.end(), i) == blocked_dirs.end()) {
            available.push_back(i);
        }
    }

    return available;
}

std::vector<std::tuple<Vector2i, int>> getNeighbouringChunks(Vector2i entityWorldCoord, World *world, Vector2i chunkCoord) {
    int chunkSize = world->get_chunk_size();
    int worldChunkWidth = world->get_chunk_width();
    int worldChunkHeight = world->get_chunk_height();

    std::vector<std::tuple<Vector2i, int>> ret = {};
    Vector2i entityLocalCoord = {
        entityWorldCoord.x % chunkSize,
        entityWorldCoord.y % chunkSize};

    if (entityLocalCoord.x == 0) {
        if (chunkCoord.x - 1 >= 0)
            ret.push_back({Vector2i(chunkCoord.x - 1, chunkCoord.y), 3});
    }
    if (entityLocalCoord.x == chunkSize - 1) {
        if (chunkCoord.x + 1 < worldChunkWidth)
            ret.push_back({Vector2i(chunkCoord.x + 1, chunkCoord.y), 1});
    }
    if (entityLocalCoord.y == 0) {
        if (chunkCoord.y - 1 >= 0)
            ret.push_back({Vector2i(chunkCoord.x, chunkCoord.y - 1), 0});
    }
    if (entityLocalCoord.y == chunkSize - 1) {
        if (chunkCoord.y + 1 < worldChunkHeight)
            ret.push_back({Vector2i(chunkCoord.x, chunkCoord.y + 1), 2});
    }

    return ret;
}

void Chunk::simulate(float delta) {
    if (entities.empty()) return;

    constexpr int MAX_STACK_TRANSFERS = 64;
    std::vector<std::shared_ptr<Entity>> to_transfer[4];

    if (entities.size() > MAX_STACK_TRANSFERS) {
        for (int i = 0; i < 4; ++i) {
            to_transfer[i].reserve(entities.size() / 16);
        }
    }

    const int chunk_size = world->get_chunk_size();
    const int world_width = world->get_world_width_tiles();
    const int world_height = world->get_world_height_tiles();

    std::vector<std::shared_ptr<Entity>> staying_entities;
    staying_entities.reserve(entities.size());

    float current_time = world->get_world_time();
    for (const auto &entity : entities) {
        if (!entity) continue;

        if (!entity->is_must_simulate()) {
            staying_entities.push_back(entity);
            continue;
        }

        if (world->is_tracking_entity_movement_per_second()) {
            float elapsed = current_time - entity->get_last_avg_update_time();
            if (entity->get_start_pos_test().x == -1) {
                entity->set_start_pos_test(entity->get_position());
            }

            if (elapsed >= 1) {
                int tiles_moved = std::abs(entity->get_start_pos_test().x - entity->get_position().x) + std::abs(entity->get_start_pos_test().y - entity->get_position().y);

                entity->set_start_pos_test(entity->get_position());
                entity->set_last_avg_update_time(current_time);

                world->increment_tiles_moved_total_fullsim(tiles_moved);
            }
        }

        Vector2i new_pos;
        Vector2i entity_world_pos = entity->get_position();

        auto neighbourChunks = getNeighbouringChunks(entity_world_pos, world, coord);

        std::vector<int> availableDirs = getAvailableDirs(entity_world_pos,
                                                           entity->get_entity_size(),
                                                           neighbourChunks);

        EntitySimulationParam params = {
            delta,
            new_pos,
            availableDirs,
            world

        };

        bool moved = entity->simulate(params);

        if (moved) {
            if (new_pos.x < 0 || new_pos.x >= world_width ||
                new_pos.y < 0 || new_pos.y >= world_height) {
                staying_entities.push_back(entity);
                continue;
            }

            entity->set_position(new_pos);
            Vector2i new_chunk = world->world_pos_to_chunk(new_pos);

            if (new_chunk == coord) {
                staying_entities.push_back(entity);
            } else {
                Vector2i direction = new_chunk - coord;

                if (direction.x != 0 && direction.y == 0) {
                    if (direction.x > 0)
                        to_transfer[1].push_back(entity);
                    else
                        to_transfer[3].push_back(entity);
                } else if (direction.y != 0 && direction.x == 0) {
                    if (direction.y > 0)
                        to_transfer[2].push_back(entity);
                    else
                        to_transfer[0].push_back(entity);
                } else {
                    if (std::abs(direction.x) > std::abs(direction.y)) {
                        if (direction.x > 0)
                            to_transfer[1].push_back(entity);
                        else
                            to_transfer[3].push_back(entity);
                    } else {
                        if (direction.y > 0)
                            to_transfer[2].push_back(entity);
                        else
                            to_transfer[0].push_back(entity);
                    }
                }
            }
        } else {
            staying_entities.push_back(entity);

        }
    }

    entities = std::move(staying_entities);

    if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1));
    if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0));
    if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1));
    if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0));
}

void Chunk::transfer_entities(std::vector<std::shared_ptr<Entity>> &entities_vec, Vector2i direction) {
    if (entities_vec.empty() || !world) return;

    Vector2i target_coord = coord + direction;

    world->request_chunk(target_coord);

    {
        std::lock_guard<std::mutex> lock(world->pending_mutex);
        for (auto &e : entities_vec) {
            world->pendingEntityPlacements.emplace_back(target_coord, e);
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
