#pragma once
#include "json.hpp"
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_map>
#include <list>
#include <string>

struct EntityData {
    bool def = true;
    int size_x = 1;
    int size_y = 1;
    float base_move_speed = 1.0f;
    int entity_type = 0;
    int entity_sprite = 0;
    bool must_simulate = false;
};

// Hash function for Vector2i to use in unordered_map
namespace std {
    template<>
    struct hash<godot::Vector2i> {
        size_t operator()(const godot::Vector2i& v) const {
            return hash<int>()(v.x) ^ (hash<int>()(v.y) << 1);
        }
    };
}

class EntityDataReader {
private:
    int maxRecentAccessSize = 100;
    
    // LRU Cache: key = Vector2i(sheet_id, sprite_id)
    std::unordered_map<godot::Vector2i, EntityData> cache;
    std::unordered_map<godot::Vector2i, std::list<godot::Vector2i>::iterator> cache_lru_map;
    std::list<godot::Vector2i> lru_list; // Most recent at front
    
    // Store which sheets have been fully loaded (optional optimization)
    std::unordered_map<int, nlohmann::json> loaded_sheets;
    
    void evict_if_needed();
    nlohmann::json load_sheet(int sheet_id);
    
public:
    EntityDataReader();
    ~EntityDataReader();
    
    // Main access method - handles cache automatically
    EntityData get_entity_data(int sheet_id, int sprite_id);
    
    // Optional: preload entire sheet into cache
    void preload_sheet(int sheet_id);
    
    // Optional: clear cache
    void clear_cache();
};
