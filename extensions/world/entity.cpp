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
    // Correct: multiply by the multiplier (0.2 = slower, 2.0 = faster)
    move_speed = base_move_speed / job.moveSpeedMultiplier;
}
