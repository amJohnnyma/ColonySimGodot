#pragma once

#include <godot_cpp/variant/vector2i.hpp>

#include <string>

struct EntityJob {
    bool isValid = false;
    bool complete = false;
    godot::Vector2i target_coord;
    std::string move_algo = "default";
    int priority = 999;
    float moveSpeedMultiplier = 1.f;
};
