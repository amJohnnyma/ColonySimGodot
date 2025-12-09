// chunk.cpp
#include "chunk.h"
#include "world.h"        // ← Full World definition here (safe!)
#include "entity.h"
#include "perlinNoise.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <random>

using namespace godot;

// === Constructor ===
Chunk::Chunk(int w, int h, Vector2i c, World* parent_world)
    : world(parent_world), coord(c), width(w), height(h) {
    tiles.resize(width * height, 0);
    tileColors.resize(width * height, Color(1,1,1,1));
}

Chunk::Chunk(int w, int h, Vector2i c) : Chunk(w, h, c, nullptr) {}

// === Generate with checkerboard entities ===
void Chunk::generate(int wx, int wy) {
    PerlinNoise noise(12345);
    BiomeType b = noise.get_biome(wx, wy);
    const float* col = BIOME_TABLE[(int)b].color;

    entities.clear();
    entities.reserve(width * height);

    int count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tiles[y * width + x] = (x + y + coord.x + coord.y) % 3;

            double n = noise.noise(
                double(x + coord.x * width) * 0.05,
                double(y + coord.y * height) * 0.05
            );
            float v = float((n + 1.0) * 0.5);
            tileColors[y * width + x] = Color(v * col[0], v * col[1], v * col[2], col[3]);

            if (count > 9) {
                Vector2 world_pos(
                    coord.x * width + x + 0.5f,
                    coord.y * height + y + 0.5f
                );
                auto e = std::make_shared<Entity>(world_pos);
                entities.push_back(e);
                count = 0;
            }
            else{
                count++;
            }
            
        }
    }
}
/*
void Chunk::simulate(float delta, bool full_simulation) {
    // N=0, E=1, S=2, W=3
    std::vector<std::shared_ptr<Entity>> to_transfer[4];
    std::vector<int> to_clear;

    for (int i = 0; i < (int)entities.size(); ++i) {
        if (!entities[i])
            continue;

        Vector2i new_pos;

        bool moved = entities[i]->simulate(
            delta,
            full_simulation ? new_pos : entities[i]->position
        );

        if (!moved)
            continue;

        // Check border crossings
        if (new_pos.x < 0) {
            to_transfer[3].push_back(entities[i]); // West
            to_clear.push_back(i);
        }
        else if (new_pos.x >= width) {
            to_transfer[1].push_back(entities[i]); // East
            to_clear.push_back(i);
        }
        else if (new_pos.y < 0) {
            to_transfer[0].push_back(entities[i]); // North
            to_clear.push_back(i);
        }
        else if (new_pos.y >= height) {
            to_transfer[2].push_back(entities[i]); // South
            to_clear.push_back(i);
        }
        else {
            // Still in this chunk
            entities[i]->position = new_pos;
        }
    }

    // ---- NEW: Queue transfers instead of applying them directly ----

    if (!to_transfer[0].empty())
        world->queue_entity_transfer(to_transfer[0], coord + Vector2i(0, -1)); // N

    if (!to_transfer[1].empty())
        world->queue_entity_transfer(to_transfer[1], coord + Vector2i(1, 0)); // E

    if (!to_transfer[2].empty())
        world->queue_entity_transfer(to_transfer[2], coord + Vector2i(0, 1)); // S

    if (!to_transfer[3].empty())
        world->queue_entity_transfer(to_transfer[3], coord + Vector2i(-1, 0)); // W

   // godot::UtilityFunctions::print("Cleared", to_clear.size());
}
*/
void Chunk::simulate(float delta, bool full_simulation) {
    return;
    std::vector<std::shared_ptr<Entity>> to_transfer[4];  // N=0, E=1, S=2, W=3
    std::vector<int> to_transfer_idx[4];  // N=0, E=1, S=2, W=3

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < (int)entities.size(); ++i) {
        if (!entities[i]) continue;
        if(entities[i]->busy) continue;
        entities[i]->busy = true;

        Vector2i new_pos;
        if (entities[i]->simulate(delta, full_simulation ? new_pos : entities[i]->position)) {
            if (new_pos.x < 0) {
                #pragma omp critical
                to_transfer[3].push_back(entities[i]);
                to_transfer_idx[3].push_back(i);
            }
            else if (new_pos.x >= width) {
                #pragma omp critical
                to_transfer[1].push_back(entities[i]);
                to_transfer_idx[1].push_back(i);
            }
            else if (new_pos.y < 0) {
                #pragma omp critical
                to_transfer[0].push_back(entities[i]);
                to_transfer_idx[0].push_back(i);
            }
            else if (new_pos.y >= height) {
                #pragma omp critical
                to_transfer[2].push_back(entities[i]);
                to_transfer_idx[2].push_back(i);
            }
            else {
                entities[i]->position = new_pos;
            }
        }
    }

    // Transfer after parallel section
    if (!to_transfer[0].empty()) transfer_entities(to_transfer[0], Vector2i(0, -1), to_transfer_idx[0]);
    if (!to_transfer[1].empty()) transfer_entities(to_transfer[1], Vector2i(1, 0), to_transfer_idx[1]);
    if (!to_transfer[2].empty()) transfer_entities(to_transfer[2], Vector2i(0, 1), to_transfer_idx[2]);
    if (!to_transfer[3].empty()) transfer_entities(to_transfer[3], Vector2i(-1, 0), to_transfer_idx[3]);
}

void Chunk::transfer_entities(std::vector<std::shared_ptr<Entity>>& entities_vec, Vector2i direction, std::vector<int> idx) {
    Vector2i target_chunk = coord + direction;
    auto target = world->get_chunk(target_chunk);
    if (!target) {
        target = world->load_chunk(target_chunk);
    }
    if (target) {
        for (auto& e : entities_vec) {
            // Wrap position into target chunk
            if (direction.x > 0) e->position.x = 0;
            if (direction.x < 0) e->position.x = width - 1;
            if (direction.y > 0) e->position.y = 0;
            if (direction.y < 0) e->position.y = height - 1;

            target->entities.push_back(e);
        }
    }
    for(auto& i : idx)
    {
        entities[i]->busy = false;
        entities[i] = nullptr;
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
