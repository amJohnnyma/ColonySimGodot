#include "colonist.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <cstdint>
#include <random>

#include "entity.h"
#include "entityJob.h"
#include "godot_cpp/core/math.hpp"

#include "godot_cpp/variant/array.hpp"
#include "world.h"

Colonist::Colonist(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size)
    : Entity(pos, id, entity_sprite, size) {
    reset_timer();
    entity_type = 1;
    homeCoord = pos;
}


bool Colonist::simulate(EntitySimulationParam &params) {
    if (!active) return false;

    move_timer -= params.delta;
    if (move_timer > 0.0f) return false;

    jobList.erase(
        std::remove_if(jobList.begin(), jobList.end(),
                       [](const EntityJob &j) { return j.complete; }),
        jobList.end());

    if (currentJobIndex >= jobList.size()) {
        currentJobIndex = -1;
    }

    if (currentJobIndex == -1 ||
        currentJobIndex < jobList.size() && jobList[currentJobIndex].complete) {
        if (!jobList.empty()) {
            auto it = std::max_element(jobList.begin(), jobList.end(),
                                       [](const EntityJob &a, const EntityJob &b) {
                                           return a.priority < b.priority;
                                       });
            currentJobIndex = std::distance(jobList.begin(), it);

            update_move_speed_from_job(jobList[currentJobIndex]);
        } else {
            EntityJob wander;
            wander.move_algo = "random";
            wander.priority = 0;
            wander.moveSpeedMultiplier = 0.2f;

            jobList.push_back(wander);
            currentJobIndex = 0;

            update_move_speed_from_job(wander);
        }
    }

    bool moved = false;
    EntityJob &current = jobList[currentJobIndex];

    update_move_speed_from_job(jobList[currentJobIndex]);
    if (current.move_algo == "default") {
        moved = default_movement(params);
    } else if (current.move_algo == "harvest") {
        moved = harvest_movement(params);
    } else {
        moved = random_movement(params);
    }

    if (current.complete) {
        reset_timer();
    }

    return moved;
}

float distance_between(const Vector2i& target, const Vector2i& pos)
{
    float distance = 0.f;
    float tsqr = std::pow((pos.x - target.x),2);
    float psqr = std::pow((pos.y - target.y),2);
    float d = tsqr + psqr;
    distance = Math::sqrt(d);

    return distance;
}

bool Colonist::harvest_movement(EntitySimulationParam &params) {
    if (currentJobIndex < 0 || currentJobIndex >= jobList.size()) {
        return false;
    }
    EntityJob &currentJob = jobList[currentJobIndex];
    Vector2i target = currentJob.target_coord;
    Vector2i current = position;

    // check if we are close to the position and then try harvest
    if (distance_between(target, current) < 2) {
        // get the entity at that pos and decrement it's inventory
        UtilityFunctions::print("Reached harvest spot");
        auto harvestable = params.world->get_entities_at_world_pos(target);

        Array types = harvestable["types"];
        if(types.size() <= 0) {
            UtilityFunctions::print("Empty spot");
            currentJob.complete = true;
            return false;
        }

        int e_type = static_cast<int>(types[0]);

        if(e_type == 4)
        {
            UtilityFunctions::print("Found harvestable. Harvesting...");
        }
        
        
        // get the entity
        // wait a second or two to harvest
        // harvest should have an infinite inventory
        // increment player inventory
        currentJob.complete = true;
        return false;
    }

    Vector2i delta = target - current;
    int dx = delta.x;
    int dy = delta.y;

    Vector2i move(0, 0);

    if (std::abs(dx) > std::abs(dy)) {
        move.x = (dx > 0) ? 1 : -1;
    } else if (std::abs(dy) > 0) {
        move.y = (dy > 0) ? 1 : -1;
    } else {
        move.x = (dx > 0) ? 1 : -1;
    }

    Vector2i desired_pos = position + move;
    if (is_position_available(desired_pos, params)) {
        params.out_new_pos = desired_pos;
        reset_timer();
        return true;
    }

    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)};

    std::vector<std::pair<int, Vector2i>> scored_dirs;
    for (const auto &dir : dirs) {
        Vector2i test_pos = position + dir;
        int score = -(std::abs(target.x - test_pos.x) + std::abs(target.y - test_pos.y));
        scored_dirs.push_back({score, dir});
    }

    std::sort(scored_dirs.begin(), scored_dirs.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });

    for (const auto &[score, dir] : scored_dirs) {
        Vector2i test_pos = position + dir;
        if (is_position_available(test_pos, params)) {
            params.out_new_pos = test_pos;
            reset_timer();
            return true;
        }
    }

    reset_timer(1.f);
    return false;
}
bool Colonist::default_movement(EntitySimulationParam &params) {
    if (currentJobIndex < 0 || currentJobIndex >= jobList.size()) {
        return false;
    }
    EntityJob &currentJob = jobList[currentJobIndex];
    Vector2i target = currentJob.target_coord;
    Vector2i current = position;

    if (current == target) {
        currentJob.complete = true;
        return false;
    }

    Vector2i delta = target - current;
    int dx = delta.x;
    int dy = delta.y;

    Vector2i move(0, 0);

    if (std::abs(dx) > std::abs(dy)) {
        move.x = (dx > 0) ? 1 : -1;
    } else if (std::abs(dy) > 0) {
        move.y = (dy > 0) ? 1 : -1;
    } else {
        move.x = (dx > 0) ? 1 : -1;
    }

    Vector2i desired_pos = position + move;
    if (is_position_available(desired_pos, params)) {
        params.out_new_pos = desired_pos;
        reset_timer();
        return true;
    }

    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)};

    std::vector<std::pair<int, Vector2i>> scored_dirs;
    for (const auto &dir : dirs) {
        Vector2i test_pos = position + dir;
        int score = -(std::abs(target.x - test_pos.x) + std::abs(target.y - test_pos.y));
        scored_dirs.push_back({score, dir});
    }

    std::sort(scored_dirs.begin(), scored_dirs.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });

    for (const auto &[score, dir] : scored_dirs) {
        Vector2i test_pos = position + dir;
        if (is_position_available(test_pos, params)) {
            params.out_new_pos = test_pos;
            reset_timer();
            return true;
        }
    }

    reset_timer(1.f);
    return false;
}

bool Colonist::random_movement(EntitySimulationParam &params) {
    jobList[currentJobIndex].complete = true;
    const Vector2i dirs[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)};

    thread_local static std::mt19937 gen(std::random_device{}());

    std::vector<int> indices = {0, 1, 2, 3};
    std::shuffle(indices.begin(), indices.end(), gen);

    for (int idx : indices) {
        Vector2i test_pos = position + dirs[idx];
        if (is_position_available(test_pos, params)) {
            params.out_new_pos = test_pos;
            reset_timer(1.f);
            return true;
        }
    }

    return false;
}
