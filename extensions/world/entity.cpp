#include "entity.h"
#include <cstdint>

Entity::Entity(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size) : position(pos), entity_id(id), entity_sprite(entity_sprite), size(size) {
    reset_timer();
}

void Entity::reset_timer() {
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(1.5f, 3.5f);
    move_timer = dist(gen);
}

