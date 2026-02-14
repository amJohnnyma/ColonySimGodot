#include "entity.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "entityJob.h"

Entity::Entity(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size)
    : position(pos), entity_id(id), entity_sprite(entity_sprite), size(size) {
    reset_timer();
}


void Entity::reset_timer(float a) {
    thread_local static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.45f, 0.55f);
    move_timer = move_speed * dist(gen) / a;
}

void Entity::add_job(EntityJob job) {
    job.isValid = true;
    job.complete = false;
    job.move_algo = "default";

    jobList.push_back(job);
}

void Entity::update_move_speed_from_job(const EntityJob &job) {
    move_speed = base_move_speed * job.moveSpeedMultiplier;
}

bool Entity::is_position_available(Vector2i pos, EntitySimulationParam &params) {
    if (params.availableDirs.empty()) {
        return false;
    }

    Vector2i move_delta = pos - position;
    int dir = -1;

    if (move_delta == Vector2i(0, -1))
        dir = 0;
    else if (move_delta == Vector2i(1, 0))
        dir = 1;
    else if (move_delta == Vector2i(0, 1))
        dir = 2;
    else if (move_delta == Vector2i(-1, 0))
        dir = 3;
    else {
        UtilityFunctions::print("WARNING: Invalid move_delta: ", move_delta.x, ",", move_delta.y);
        return false;
    }

    bool is_available = std::find(params.availableDirs.begin(),
                                   params.availableDirs.end(), dir) != params.availableDirs.end();

    return is_available;
}
