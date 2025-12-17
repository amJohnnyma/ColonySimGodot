// entity.h
#pragma once
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
        Entity(Vector2i pos);

    public:
        virtual ~Entity() = default;

        virtual bool simulate(EntitySimulationParam &params) = 0; 

    public: // Common
        void reset_timer(); 
        Vector2i get_position() { return position; }
        void set_position(Vector2i pos) { position = pos; }
        bool is_active() { return active; }

};
