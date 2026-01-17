#include "entity.h"
#include "entityJob.h"
#include <algorithm>
#include <cstdint>
#include <cmath>

Entity::Entity(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size) : position(pos), entity_id(id), entity_sprite(entity_sprite), size(size) {
    reset_timer();
}

void Entity::reset_timer() {
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.9f, 1.1f);
    move_timer = move_speed * dist(gen);
}

// this should be an override -> move speed for items and buildings ???? Should defnitely be removed from Entity base
void Entity::add_job(EntityJob job)
{
    job.isValid = true;
    job.complete = false;
    job.move_algo = "default";


    jobList.push_back(job);
}
void Entity::update_move_speed_from_job(const EntityJob& job)
{
    // multiply by the multiplier (0.2 = slower, 2.0 = faster)
    move_speed = base_move_speed / job.moveSpeedMultiplier;
}

// Helper method to check if a position is available (add to colonist.h)
bool Entity::is_position_available(Vector2i pos, EntitySimulationParam &params)
{// If no collision data provided, assume available
 //
    if (params.availableDirs.empty())
    {
        UtilityFunctions::print("No availableDirs data, assuming position is available");
        return true;
    }
    
    Vector2i move_delta = pos - position;
    int dir = -1;
    
    // Map movement to direction index
    if (move_delta == Vector2i(0, -1)) dir = 0; // North
    else if (move_delta == Vector2i(1, 0)) dir = 1; // East
    else if (move_delta == Vector2i(0, 1)) dir = 2; // South
    else if (move_delta == Vector2i(-1, 0)) dir = 3; // West
    else {
        UtilityFunctions::print("WARNING: Invalid move_delta: ", move_delta.x, ",", move_delta.y);
        return false; // Invalid movement
    }
    
    bool is_available = std::find(params.availableDirs.begin(), 
                                   params.availableDirs.end(), dir) != params.availableDirs.end();
    
    //UtilityFunctions::print("Checking dir ", dir, " -> ", (is_available ? "available" : "blocked"));
    
    return is_available;
}
