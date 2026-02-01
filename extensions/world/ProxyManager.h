#pragma once
#include <godot_cpp/variant/vector2i.hpp>
#include "entity.h"
#include "colonist.h"
#include "entityJob.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include <memory>
#include <vector>
#include <algorithm>

struct ProxyEntity {
    uint64_t entity_id = 0;
    Vector2i position;
    double last_full_sim_time = 0.0;
    double next_proxy_update_time = 0.0;

    // Full job list
    std::vector<EntityJob> job_list;
    int current_job_index = -1;

    // Entity construction data
    int entity_sprite = 0;
    Vector2i entity_size = Vector2i(1, 1);
    Vector2i home_coord = Vector2i(0, 0);
    float base_move_speed = 1.0f;
};

class ProxyManager {
    public:
        float random_float(float min, float max) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }

        ProxyEntity entity_to_proxy(const std::shared_ptr<Entity>& entity, double world_time) {
            ProxyEntity proxy;
            if (!entity) return proxy;

            proxy.entity_id = entity->get_entity_id();
            proxy.position = entity->get_position();
            proxy.last_full_sim_time = world_time;
          //  UtilityFunctions::print("Created proxy with last sim time ", world_time);

            if (auto colonist = std::dynamic_pointer_cast<Colonist>(entity)) {
                proxy.entity_sprite = colonist->get_entity_sprite();
                proxy.entity_size = colonist->get_entity_size();
                proxy.home_coord = colonist->get_home_coord();
                proxy.base_move_speed = colonist->get_base_move_speed();

                // Copy the entire job list
                proxy.job_list = colonist->get_job_list();
                proxy.current_job_index = colonist->get_current_job_index();

                // If no jobs, create a wander job like in Colonist::simulate
                if (proxy.job_list.empty()) {
                    EntityJob wander;
                    wander.move_algo = "random";
                    wander.priority = 0;
                    wander.moveSpeedMultiplier = 0.2f;
                    wander.complete = false;
                    proxy.job_list.push_back(wander);
                    proxy.current_job_index = 0;
                }
                // If current job index is invalid, find highest priority job
                else if (proxy.current_job_index == -1 || 
                        proxy.current_job_index >= proxy.job_list.size() ||
                        proxy.job_list[proxy.current_job_index].complete) {

                    auto it = std::max_element(proxy.job_list.begin(), proxy.job_list.end(),
                            [](const EntityJob& a, const EntityJob& b) {
                            return a.priority < b.priority;
                            });
                    int index = std::distance(proxy.job_list.begin(), it);
                    proxy.current_job_index = index; 
                }
            }
            proxy.next_proxy_update_time = world_time + random_float(0.0f, 0.6f);

            return proxy;
        }
        Vector2i extrapolate_position(const ProxyEntity& proxy, double current_time) {
            double elapsed = current_time - proxy.last_full_sim_time;


            // Check if we have a valid job
            if (proxy.job_list.empty() || 
                    proxy.current_job_index < 0 || 
                    proxy.current_job_index >= proxy.job_list.size()) {
                return proxy.position;
            }

            const EntityJob& current_job = proxy.job_list[proxy.current_job_index];

            // Don't move wandering entities or those without valid targets
            if (current_job.move_algo == "random" || current_job.target_coord.x == -1) {
                return proxy.position;
            }

            // Calculate movement speed for this job
            float move_speed = proxy.base_move_speed / current_job.moveSpeedMultiplier;
            float distance_traveled = move_speed * elapsed;

            Vector2i delta = current_job.target_coord - proxy.position;
            int abs_dx = std::abs(delta.x);
            int abs_dy = std::abs(delta.y);
            int total_distance = abs_dx + abs_dy;

            if (total_distance == 0) return proxy.position;

            int tiles_moved = std::min((int)distance_traveled, total_distance);

            // O(1) calculation instead of O(tiles_moved) loop
            // The default_movement prioritizes the larger axis
            // We can calculate how the tiles are distributed mathematically

            Vector2i new_pos = proxy.position;

            if (tiles_moved >= total_distance) {
                // Reached or passed the target
                return current_job.target_coord;
            }

            // Calculate how tiles are distributed between x and y
            // The algorithm moves along the larger axis more frequently
            int larger_axis_moves, smaller_axis_moves;
            bool x_is_larger = abs_dx > abs_dy;

            if (x_is_larger) {
                // X is prioritized - zigzag pattern
                // For every abs_dy moves, we do abs_dx horizontal and abs_dy vertical
                // Pattern repeats every (abs_dx + abs_dy) moves
                larger_axis_moves = std::min(tiles_moved, abs_dx);
                smaller_axis_moves = std::max(0, tiles_moved - abs_dx);
            } else {
                // Y is prioritized
                larger_axis_moves = std::min(tiles_moved, abs_dy);
                smaller_axis_moves = std::max(0, tiles_moved - abs_dy);
            }

            if (x_is_larger) {
                new_pos.x += (delta.x > 0 ? larger_axis_moves : -larger_axis_moves);
                new_pos.y += (delta.y > 0 ? smaller_axis_moves : -smaller_axis_moves);
            } else {
                new_pos.y += (delta.y > 0 ? larger_axis_moves : -larger_axis_moves);
                new_pos.x += (delta.x > 0 ? smaller_axis_moves : -smaller_axis_moves);
            }


            UtilityFunctions::print("Proxy extrapolate entity: ", proxy.entity_id, "\tpos ", proxy.position, " to ", new_pos, " with move speed=", move_speed);

            return new_pos;
        }


        std::shared_ptr<Entity> proxy_to_entity(const ProxyEntity& proxy, double current_time) {
            Vector2i extrapolated_pos = extrapolate_position(proxy, current_time);

            // Construct with proper parameters
            auto entity = std::make_shared<Colonist>(
                    extrapolated_pos,
                    proxy.entity_id,
                    proxy.entity_sprite,
                    proxy.entity_size
                    );

            // Restore home coordinate
            entity->set_home_coord(proxy.home_coord);

            // Restore the entire job list
            if (!proxy.job_list.empty()) {
                // Update job completion status based on extrapolated position
                std::vector<EntityJob> updated_jobs = proxy.job_list;

                for (size_t i = 0; i < updated_jobs.size(); ++i) {
                    // Mark jobs as complete if we've reached their target
                    if (updated_jobs[i].target_coord != Vector2i(-1, -1) &&
                            extrapolated_pos == updated_jobs[i].target_coord) {
                        updated_jobs[i].complete = true;
                    }
                }

                // Set the job list
                entity->set_job_list(updated_jobs);

                // Set current job index (the entity will handle finding the next highest priority on next simulate)
                entity->set_current_job_index(proxy.current_job_index);
            }

            return entity;
        }
};
