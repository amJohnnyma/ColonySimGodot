#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <cstdint>
#include <random>
#include <vector>

#include "entityJob.h"

using namespace godot;

struct EntitySimulationParam {
    float delta;
    Vector2i &out_new_pos;
    std::vector<int> availableDirs;
};

class Entity {
protected:
    int currentJobIndex = -1;
    std::vector<EntityJob> jobList;
    Vector2i position;
    float move_timer = 0.0f;
    bool active = true;
    uint64_t entity_id;
    int entity_type;
    int entity_sprite;
    Vector2i size = Vector2i(1, 1);
    float move_speed = 0.f;
    float base_move_speed = 1.0f;
    float tiles_per_second_test = 0.0f;
    int seconds_elapsed_test = 0;
    int tiles_moved = 0;
    float last_avg_update_time = 0.0f;
    Vector2i start_pos_test = Vector2i(-1, -1);
    bool must_simulate = true;

    Entity(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size = Vector2i(1, 1));
    Entity() {}; // plain entity // current use -> deserialization

public:
    virtual ~Entity() = default;
    virtual bool simulate(EntitySimulationParam &params) = 0;
    virtual int get_type_id() const {return entity_type;};
    void set_type_id(int type) {entity_type = type;}

    void reset_timer(float a = 1);
    Vector2i get_position() { return position; }
    void set_position(Vector2i pos) { position = pos; }
    bool is_active() { return active; }
    void set_entity_id(int id) {entity_id = id;}
    uint64_t get_entity_id() const { return entity_id; }
    int get_entity_sprite() const { return entity_sprite; }
    Vector2i get_entity_size() const { return size; }
    int get_entity_width() const { return size.x; }
    int get_entity_height() const { return size.y; }
    void set_move_speed(float speed) { move_speed = speed; }
    float get_move_speed() { return move_speed; }
    float get_base_move_speed() { return base_move_speed; }
    void set_base_move_speed(float speed) { base_move_speed = speed; }
    int get_current_job_index() { return currentJobIndex; }
    bool has_active_job() const { return currentJobIndex != -1 && currentJobIndex < jobList.size(); }
    bool is_must_simulate() { return must_simulate; }
    void set_must_simulate(bool flag) { must_simulate = flag; }
    void set_entity_sprite(int sprite) { entity_sprite = sprite; }
    void set_entity_size(Vector2i entity_size) { size = entity_size; }
    void set_active(bool is_active) { active = is_active; }

    void update_move_speed_from_job(const EntityJob &job);
    void add_job(EntityJob job);
    bool is_position_available(Vector2i pos, EntitySimulationParam &params);

    std::vector<EntityJob> get_job_list() const { return jobList; }
    void set_job_list(const std::vector<EntityJob> &jobs) { jobList = jobs; }
    void set_current_job_index(int index) { currentJobIndex = index; }
    EntityJob get_current_job() const {
        if (has_active_job()) return jobList[currentJobIndex];
        return EntityJob();
    }

    float get_tiles_per_second_test() const { return tiles_per_second_test; }
    void set_tiles_per_second_test(float value) { tiles_per_second_test = value; }
    int get_seconds_elapsed_test() const { return seconds_elapsed_test; }
    void set_seconds_elapsed_test(int value) { seconds_elapsed_test = value; }
    int get_tiles_moved() const { return tiles_moved; }
    void set_tiles_moved(int value) { tiles_moved = value; }
    float get_last_avg_update_time() const { return last_avg_update_time; }
    void set_last_avg_update_time(float value) { last_avg_update_time = value; }
    Vector2i get_start_pos_test() const { return start_pos_test; }
    void set_start_pos_test(const Vector2i &value) { start_pos_test = value; }
};
