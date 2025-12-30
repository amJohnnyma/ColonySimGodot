#include "entity.h"
#include <cstdint>


class Item : public Entity
{

    public:
        Item(Vector2i pos, uint64_t id,int entity_sprite, Vector2i size);
        bool simulate(EntitySimulationParam &params) override;
        int get_type_id() const override { return entity_type; }
};
