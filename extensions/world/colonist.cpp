#include "colonist.h"
#include "entity.h"
#include "entityJob.h"
#include "variant/vector2i.hpp"
#include <cstdint>
#include <random>

Colonist::Colonist(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size) : Entity(pos, id, entity_sprite, size) {
    reset_timer();
    entity_type = 1;
    homeCoord = pos;

}
// Returns new position and true if crossed chunk border
bool Colonist::simulate(EntitySimulationParam &params)
{
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;

    // Clean up completed jobs
    jobList.erase(
        std::remove_if(jobList.begin(), jobList.end(),
            [](const EntityJob& j) { return j.complete || j.priority == 0; }),
        jobList.end()
    );

    if (currentJobIndex >= jobList.size()) { currentJobIndex = -1; }

    // If no current job or current one is complete/nullptr
    if (currentJobIndex == -1 ||
            currentJobIndex < jobList.size() && jobList[currentJobIndex].complete)
    {
        if (!jobList.empty())
        {
            // Pick highest priority job
            auto it = std::max_element(jobList.begin(), jobList.end(),
                [](const EntityJob& a, const EntityJob& b) {
                    return a.priority < b.priority;
                });
            currentJobIndex = std::distance(jobList.begin(), it);

            update_move_speed_from_job(jobList[currentJobIndex]);
        }
        else
        {
            // Create new wander job (uncommented to allow wandering)
            EntityJob wander;
            wander.move_algo = "random";
            wander.priority = 0;
            wander.moveSpeedMultiplier = 0.2f; // move at 20% base speed
                                               //
            jobList.push_back(wander);

            update_move_speed_from_job(wander);
        }
    }

    // Execute movement
    bool moved = false;
    EntityJob& current = jobList[currentJobIndex];        
    if (current.move_algo == "default")
    {
        moved = default_movement(params);
    }
    else
    {
        moved = random_movement(params);
    }

    // If job completed this tick, reset timer and let cleanup handle removal next frame
    if (current.complete)
    {
        reset_timer();
    }

    return moved;
}
bool Colonist::default_movement(EntitySimulationParam &params)
{
    if(currentJobIndex < 0 || currentJobIndex >= jobList.size()) { return false; }
    EntityJob& currentJob = jobList[currentJobIndex];
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
