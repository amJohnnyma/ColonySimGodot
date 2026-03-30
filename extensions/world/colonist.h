#pragma once

#include <godot_cpp/variant/vector2i.hpp>

#include <algorithm>
#include <cstdint>

#include "entity.h"

class Colonist : public Entity {
private:
    Vector2i homeCoord;

public:
    Colonist(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size);
    Colonist() : Entity(){};
    bool simulate(EntitySimulationParam &params) override;
    int get_type_id() const override { return entity_type; }
    bool default_movement(EntitySimulationParam &params);
    bool harvest_movement(EntitySimulationParam &params);
    bool random_movement(EntitySimulationParam &params);
    Vector2i get_home_coord() { return homeCoord; }
    void set_home_coord(Vector2i coord) { homeCoord = coord; }
};
