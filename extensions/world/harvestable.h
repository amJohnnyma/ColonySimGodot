
#pragma once

#include <cstdint>

#include "entity.h"

class Harvestable: public Entity {
public:
    Harvestable(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size);
    Harvestable() : Entity() {};
    bool simulate(EntitySimulationParam &params) override;
    int get_type_id() const override { return entity_type; }
};
