// entity.h
#pragma once
#include <godot_cpp/variant/vector2i.hpp>
#include <random>

using namespace godot;

struct Entity {
    Vector2i position;           // Grid position (local to chunk!)
    float move_timer = 0.0f;
    bool active = true;

    Entity(Vector2i pos) : position(pos) {
        reset_timer();
    }

    void reset_timer() {
        thread_local static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(1.5f, 3.5f);
        move_timer = dist(gen);
    }

    // Returns new position and true if crossed chunk border
    bool simulate(float delta, Vector2i& out_new_pos, std::vector<int> availableDirs) {
        if (!active) return false;

        move_timer -= delta;
        if (move_timer > 0.0f) return false;



        const Vector2i dirs[4] = {
            Vector2i(0, -1), Vector2i(1, 0),
            Vector2i(0, 1), Vector2i(-1, 0)
        };
        thread_local static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 3);

        out_new_pos = position + dirs[dist(gen)];
        reset_timer();
        return true;  // Moved
    }
};
/*
 *
    bool simulate(float delta, Vector2i& out_new_pos, std::vector<int> availableDirs) {
        if (!active) return false;

        move_timer -= delta;
        if (move_timer > 0.0f) return false;

        if (availableDirs.empty()) {
            out_new_pos = position;      // don't move
            reset_timer();               // still consume the turn (prevents spamming)
            return false;
        }

        thread_local static std::mt19937 rng{std::random_device{}()};

        // If there's only one option, just use it (common case)
        int chosen_dir_index;
        if (availableDirs.size() == 1) {
            chosen_dir_index = 0;
        } else {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(availableDirs.size() - 1));
            chosen_dir_index = dist(rng);
        }

        int chosen_dir = availableDirs[chosen_dir_index];

        const Vector2i offsets[4] = {
        Vector2i(0, -1), Vector2i(1, 0),
        Vector2i(0, 1), Vector2i(-1, 0)
        };

        out_new_pos = position + offsets[chosen_dir];

        reset_timer();
        return true; // we actually moved

    }
};
 * */

/*
// entity.h
#pragma once
#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

struct Entity {
    Vector2i position;           
    float move_timer = 0.0f;
    bool active = true;
    bool busy = false; // for parallel

    int step_index = 0; // deterministic movement state

    Entity(Vector2i pos) : position(pos) {
        reset_timer();
    }

    void reset_timer() {
        move_timer = 1.0f; // fixed update interval
    }

    // Returns true if movement happened
    bool simulate(float delta, Vector2i& out_new_pos) {
        if (!active) return false;

        // count down to movement
        move_timer -= delta;
        if (move_timer > 0.0f) {
            out_new_pos = position;
            return false;
        }

        // deterministic directions:
        // 0 = right, 1 = down, 2 = left, 3 = up
        static const Vector2i dirs[4] = {
            Vector2i(1, 0),
            Vector2i(0, 1),
            Vector2i(-1, 0),
            Vector2i(0, -1)
        };

        out_new_pos = position + dirs[step_index];

        // cycle through pattern
        step_index = (step_index + 1) % 4;

        // reset timer
        reset_timer();

        return true;
    }
};*/
