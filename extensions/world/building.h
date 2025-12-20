#include "entity.h"
#include <cstdint>


class Building : public Entity
{
    private:
        int type;// just a string for now

    public:
        Building(Vector2i pos, uint64_t id, int type);
        bool simulate(EntitySimulationParam &params) override;
        int get_type_id() const override { return entity_type; }
};
