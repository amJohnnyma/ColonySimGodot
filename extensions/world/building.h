#include "entity.h"


class Building : public Entity
{
    private:

    public:
        Building(Vector2i pos);
        bool simulate(EntitySimulationParam &params) override;
};
