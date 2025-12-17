#include "entity.h"
class Colonist : public Entity
{
    private:

    public:
        Colonist(Vector2i pos);
        bool simulate(EntitySimulationParam &params) override;
};
