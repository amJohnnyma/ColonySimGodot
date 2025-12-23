#include "building.h"
#include "entity.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdint>

Building::Building(Vector2i pos, uint64_t id, int type) : Entity(pos, id), type(type) {
    reset_timer();
    entity_type = 1;
}

bool Building::simulate(EntitySimulationParam &params) {
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;

    //UtilityFunctions::print("Building");
    reset_timer();
    return false;  // Never moves 
}
