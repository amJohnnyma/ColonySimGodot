#include "colonist.h"
#include "entity.h"
#include "entityJob.h"
#include "variant/vector2i.hpp"
#include <cstdint>

Colonist::Colonist(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size) : Entity(pos, id, entity_sprite, size) {
    reset_timer();
    entity_type = 1;
    homeCoord = pos;

    // TEMP JOB ON INIT. MOVE TO 0,0
    EntityJob job;
    job.isValid = true;
    job.complete = false;
    job.move_algo = "default";
    job.priority = 0;
    job.target_coord = Vector2i(0,0);

    currentJob = job;
}
// Returns new position and true if crossed chunk border
bool Colonist::simulate(EntitySimulationParam &params) {
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;
    
    if(!currentJob.isValid)
    {
        for(auto& job : jobList)
        {
           if(job.isValid)
           {
               currentJob = job;
           } 
        }
    }

    if(!currentJob.isValid)
    {
        return false;
    }
    if(currentJob.complete)
    {
        reset_timer();
        return false;
    }

    std::string move_algo = currentJob.move_algo;

    
    if(move_algo == "default")
    {
        return default_movement(params);
    }
    else if(move_algo == "random")
    {
        return random_movement(params);
    }
    else
    {
        return random_movement(params);
    }




}

bool Colonist::default_movement(EntitySimulationParam &params)
{
    Vector2i target = currentJob.target_coord;
    Vector2i current = position;

    // If already at target, mark job complete
    if (current == target)
    {
        currentJob.complete = true;
        return false; // No movement needed
    }

    // Compute direction vector
    Vector2i delta = target - current;
    int dx = delta.x;
    int dy = delta.y;

    // This reduces total steps needed
    Vector2i move(0, 0);

    if (std::abs(dx) > std::abs(dy))
    {
        // Move horizontally first
        move.x = (dx > 0) ? 1 : -1;
    }
    else if (std::abs(dy) > 0)
    {
        // Move vertically (if no horizontal needed, or tie)
        move.y = (dy > 0) ? 1 : -1;
    }
    else
    {
        // If somehow both zero but not equal (shouldn't happen), try horizontal
        move.x = (dx > 0) ? 1 : -1;
    }


    params.out_new_pos = position + move;
    reset_timer();

    return true; // Successfully moved toward target
}

bool Colonist::random_movement(EntitySimulationParam &params)
{
    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)
    };
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 3);

    params.out_new_pos = position + dirs[dist(gen)];
    reset_timer();
    return true;  // Moved
}
