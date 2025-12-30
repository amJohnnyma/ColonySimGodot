#include "entity.h"
#include "variant/vector2i.hpp"
#include <cstdint>
class Colonist : public Entity
{
    private:

    public:
        Colonist(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size);
        bool simulate(EntitySimulationParam &params) override;
        int get_type_id() const override { return entity_type; }
};
