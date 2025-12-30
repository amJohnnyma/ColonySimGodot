#include "item.h"
#include "entity.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdint>

Item::Item(Vector2i pos, uint64_t id, int entity_sprite,Vector2i size) : Entity(pos, id, entity_sprite, size) {
    reset_timer();
    entity_type = 3;
}

bool Item::simulate(EntitySimulationParam &params) {
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;

    //UtilityFunctions::print("Building");
    reset_timer();
    return false;  // Never moves 
}
