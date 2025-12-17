#include "entity.h"
#include <cstdint>


class Building : public Entity
{
    private:
        std::string type;// just a string for now

    public:
        Building(Vector2i pos, uint64_t id, std::string type);
        bool simulate(EntitySimulationParam &params) override;
        int get_type_id() const override { return 1; }
};
