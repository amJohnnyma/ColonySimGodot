#include "building.h"
#include "entity.h"
#include <cstdint>

Building::Building(Vector2i pos, uint64_t id, std::string type) : Entity(pos, id), type(type) {
    reset_timer();
    entity_type = 1;
}

bool Building::simulate(EntitySimulationParam &params) {
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;



    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)
    };
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 3);

    UtilityFunctions::print("Building simulated - ", dist(gen));
    reset_timer();
    return true;  // Moved
}
