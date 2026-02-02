#include "entityDataReader.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <fstream>
#include <iostream>

EntityDataReader::EntityDataReader() {}
EntityDataReader::~EntityDataReader() {}

nlohmann::json EntityDataReader::load_sheet(int sheet_id) {
    auto it = loaded_sheets.find(sheet_id);
    if (it != loaded_sheets.end()) {
        return it->second;
    }

    godot::String godot_path = godot::String("res://EntityData/sprite_sheet_") +
                               godot::String::num_int64(sheet_id) + ".json";

    godot::Ref<godot::FileAccess> file = godot::FileAccess::open(godot_path, godot::FileAccess::READ);

    if (file.is_null() || !file->is_open()) {
        godot::UtilityFunctions::printerr("Could not open: ", godot_path);
        return nlohmann::json::object();
    }

    godot::String json_string = file->get_as_text();
    file->close();

    nlohmann::json j;
    try {
        std::string std_json = json_string.utf8().get_data();
        j = nlohmann::json::parse(std_json);
    } catch (const nlohmann::json::exception &e) {
        godot::UtilityFunctions::printerr("JSON parse error: ", e.what());
        return nlohmann::json::object();
    }

    loaded_sheets[sheet_id] = j;
    return j;
}

void EntityDataReader::evict_if_needed() {
    if (cache.size() >= maxRecentAccessSize) {
        auto lru_key = lru_list.back();
        lru_list.pop_back();
        cache.erase(lru_key);
        cache_lru_map.erase(lru_key);
    }
}

EntityData EntityDataReader::get_entity_data(int sheet_id, int sprite_id) {
    godot::Vector2i key(sheet_id, sprite_id);

    auto cache_it = cache.find(key);
    if (cache_it != cache.end()) {
        lru_list.erase(cache_lru_map[key]);
        lru_list.push_front(key);
        cache_lru_map[key] = lru_list.begin();

        return cache_it->second;
    }

    nlohmann::json sheet = load_sheet(sheet_id);

    std::string sprite_key = std::to_string(sprite_id);
    if (!sheet.contains(sprite_key)) {
        std::cerr << "Warning: Sprite " << sprite_id << " not found in sheet " << sheet_id << std::endl;
        return EntityData();
    }

    EntityData data;
    auto sprite = sheet[sprite_key];

    data.size_x = sprite.value("size_x", 1);
    data.size_y = sprite.value("size_y", 1);
    data.base_move_speed = sprite.value("base_move_speed", 1.0f);
    data.entity_type = sprite.value("entity_type", 0);
    data.entity_sprite = sprite.value("entity_sprite", 0);
    data.must_simulate = sprite.value("must_simulate", false);

    data.def = false;

    evict_if_needed();
    cache[key] = data;
    lru_list.push_front(key);
    cache_lru_map[key] = lru_list.begin();

    return data;
}

void EntityDataReader::preload_sheet(int sheet_id) {
    nlohmann::json sheet = load_sheet(sheet_id);

    for (auto &[key, value] : sheet.items()) {
        int sprite_id = std::stoi(key);
        get_entity_data(sheet_id, sprite_id);
    }
}

void EntityDataReader::clear_cache() {
    cache.clear();
    cache_lru_map.clear();
    lru_list.clear();
    loaded_sheets.clear();
}
