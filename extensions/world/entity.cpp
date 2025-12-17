#include "entity.h"

Entity::Entity(Vector2i pos) : position(pos) {
    reset_timer();
}

void Entity::reset_timer() {
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(1.5f, 3.5f);
    move_timer = dist(gen);
}

