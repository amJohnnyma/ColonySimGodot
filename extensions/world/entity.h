#pragma once
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

struct Entity {
    Vector2i position;
    bool active;

    Entity(Vector2i pos, bool act = true) : position(pos), active(act) {}

    void simulate(float delta) {
        // Minimal simulation placeholder
    }
};
