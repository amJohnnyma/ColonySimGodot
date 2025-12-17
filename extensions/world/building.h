#include "entity.h"


class Building : public Entity
{
    private:
        std::string type;// just a string for now

    public:
        Building(Vector2i pos, std::string type);
        bool simulate(EntitySimulationParam &params) override;
};
