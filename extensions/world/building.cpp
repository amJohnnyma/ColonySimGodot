#include "building.h"

#include <cstdint>

#include "entity.h"

Building::Building(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size)
    : Entity(pos, id, entity_sprite, size) {
    reset_timer();
    entity_type = 2;
}

bool Building::simulate(EntitySimulationParam &params) {
    if (!active) return false;
    if (!must_simulate) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;

    reset_timer();
    return false;
}
