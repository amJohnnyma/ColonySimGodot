#include "item.h"
#include "entity.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdint>

Item::Item(Vector2i pos, uint64_t id, int entity_sprite,Vector2i size) : Entity(pos, id, entity_sprite, size) {
    entity_type = 3;
}

bool Item::simulate(EntitySimulationParam &params) {
    return false;  // Never moves 
}
