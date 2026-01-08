
#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/color.hpp>
#include <iostream>

struct EntityJob
{
    // to check if constructed
    bool isValid = false;

        // actual vars
    bool complete = false;
    godot::Vector2i target_coord;
    std::string move_algo = "default";
    int priority = 999;
    float moveSpeedMultiplier = 1.f;

};
