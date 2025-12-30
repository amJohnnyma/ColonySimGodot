// entity.h
#pragma once
#include <cstdint>
#include <godot_cpp/variant/vector2i.hpp>
#include <random>


#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

using namespace godot;

struct EntitySimulationParam
{
    float delta; 
    Vector2i& out_new_pos;
    std::vector<int> availableDirs;

};

class Entity 
{
    protected:
        Vector2i position;           // Grid position (local to chunk!)
        float move_timer = 0.0f;
        bool active = true;
        Entity(Vector2i pos, uint64_t id, int entity_sprite, Vector2i size = Vector2i(1,1));
        uint64_t entity_id;
        int entity_type;
        int entity_sprite;
        Vector2i size = Vector2i(1,1);
        float move_speed = 2.f;
    public:
        virtual ~Entity() = default;

        virtual bool simulate(EntitySimulationParam &params) = 0; 

    public: // Common
        void reset_timer(); 
        Vector2i get_position() { return position; }
        void set_position(Vector2i pos) { position = pos; }
        bool is_active() { return active; }
        virtual int get_type_id() const = 0;
        uint64_t get_entity_id() const { return entity_id; }
        int get_entity_sprite() const { return entity_sprite; }
        Vector2i get_entity_size() const { return size; }
        int get_entity_width() const { return size.x; }
        int get_entity_height() const { return size.y; }
        void set_move_speed(float speed) {move_speed = speed;}


};
