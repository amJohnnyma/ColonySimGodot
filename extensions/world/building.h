#pragma once

#include <cstdint>

#include "entity.h"

class Building : public Entity {
public:
    Building(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size);
    bool simulate(EntitySimulationParam &params) override;
    int get_type_id() const override { return entity_type; }
};
