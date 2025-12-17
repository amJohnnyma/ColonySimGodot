#include "entity.h"
#include <cstdint>
class Colonist : public Entity
{
    private:

    public:
        Colonist(Vector2i pos, uint64_t id);
        bool simulate(EntitySimulationParam &params) override;
        int get_type_id() const override { return entity_type; }
};
