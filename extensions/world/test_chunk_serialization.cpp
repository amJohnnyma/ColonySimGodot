// test_chunk_serialization_cout.cpp
// Uses std::cout instead of UtilityFunctions to avoid Godot initialization

#include "chunk_manager.h"
#include "chunk.h"
#include "entity.h"
#include "colonist.h"
#include "entityJob.h"
#include <iostream>
#include <cmath>

using namespace godot;

// Replace UtilityFunctions with cout macros
#define PRINT(x) std::cout << x << std::endl
#define PRINTERR(x) std::cerr << x << std::endl

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/godot.hpp>

// Add these before main():
extern "C" {
    // Minimal GDExtension initialization for standalone testing
    GDExtensionInterfaceGetGodotVersion get_godot_version;
    GDExtensionInterfaceMemAlloc mem_alloc;
    GDExtensionInterfaceMemRealloc mem_realloc;
    GDExtensionInterfaceMemFree mem_free;
    GDExtensionInterfacePrintError print_error;
    GDExtensionInterfacePrintWarning print_warning;
    GDExtensionInterfacePrintScriptError print_script_error;
    
    void simple_mem_alloc(void *p_mem, size_t p_bytes) {
        *(void **)p_mem = malloc(p_bytes);
    }
    
    void simple_mem_realloc(void *p_mem, void *p_ptr, size_t p_bytes) {
        *(void **)p_mem = realloc(p_ptr, p_bytes);
    }
    
    void simple_mem_free(void *p_mem) {
        free(p_mem);
    }
}

// Comparison functions
bool compare_entity_jobs(const std::vector<EntityJob>& a, const std::vector<EntityJob>& b) {
    if (a.size() != b.size()) {
        PRINTERR("Job count mismatch: " << a.size() << " vs " << b.size());
        return false;
    }
    
    const float epsilon = 0.0001f;
    for (size_t i = 0; i < a.size(); ++i) {
        const EntityJob& ja = a[i];
        const EntityJob& jb = b[i];
        
        if (ja.isValid != jb.isValid) {
            PRINTERR("Job " << i << " isValid mismatch");
            return false;
        }
        if (ja.complete != jb.complete) {
            PRINTERR("Job " << i << " complete mismatch");
            return false;
        }
        if (ja.target_coord != jb.target_coord) {
            PRINTERR("Job " << i << " target_coord mismatch");
            return false;
        }
        if (ja.move_algo != jb.move_algo) {
            PRINTERR("Job " << i << " move_algo mismatch: '" << ja.move_algo << "' vs '" << jb.move_algo << "'");
            return false;
        }
        if (ja.priority != jb.priority) {
            PRINTERR("Job " << i << " priority mismatch");
            return false;
        }
        if (std::abs(ja.moveSpeedMultiplier - jb.moveSpeedMultiplier) > epsilon) {
            PRINTERR("Job " << i << " moveSpeedMultiplier mismatch");
            return false;
        }
    }
    
    return true;
}

bool compare_entities(Entity* a, Entity* b) {
    if (!a || !b) {
        PRINTERR("One or both entities are null");
        return false;
    }
    
    if (a->get_type_id() != b->get_type_id()) {
        PRINTERR("Entity type mismatch: " << a->get_type_id() << " vs " << b->get_type_id());
        return false;
    }
    
    if (a->get_entity_id() != b->get_entity_id()) {
        PRINTERR("Entity ID mismatch");
        return false;
    }
    
    if (a->get_position() != b->get_position()) {
        PRINTERR("Entity position mismatch");
        return false;
    }
    
    if (a->get_entity_size() != b->get_entity_size()) {
        PRINTERR("Entity size mismatch");
        return false;
    }
    
    if (a->get_entity_sprite() != b->get_entity_sprite()) {
        PRINTERR("Entity sprite mismatch");
        return false;
    }
    
    const float epsilon = 0.0001f;
    if (std::abs(a->get_move_speed() - b->get_move_speed()) > epsilon) {
        PRINTERR("Entity move_speed mismatch");
        return false;
    }
    
    if (std::abs(a->get_base_move_speed() - b->get_base_move_speed()) > epsilon) {
        PRINTERR("Entity base_move_speed mismatch");
        return false;
    }
    
    if (a->is_active() != b->is_active()) {
        PRINTERR("Entity active flag mismatch");
        return false;
    }
    
    if (a->is_must_simulate() != b->is_must_simulate()) {
        PRINTERR("Entity must_simulate flag mismatch");
        return false;
    }
    
    if (a->get_current_job_index() != b->get_current_job_index()) {
        PRINTERR("Entity job index mismatch");
        return false;
    }
    
    if (!compare_entity_jobs(a->get_job_list(), b->get_job_list())) {
        return false;
    }
    
    if (a->get_type_id() == 1) {
        Colonist* col_a = dynamic_cast<Colonist*>(a);
        Colonist* col_b = dynamic_cast<Colonist*>(b);
        
        if (col_a && col_b) {
            if (col_a->get_home_coord() != col_b->get_home_coord()) {
                PRINTERR("Colonist home_coord mismatch");
                return false;
            }
        }
    }
    
    return true;
}

bool compare_chunks(const Chunk* a, const Chunk* b) {
    if (!a || !b) {
        PRINTERR("One or both chunks are null");
        return false;
    }
    
    if (a->coord != b->coord) {
        PRINTERR("Coord mismatch");
        return false;
    }
    
    if (a->width != b->width || a->height != b->height) {
        PRINTERR("Size mismatch");
        return false;
    }
    
    if (a->tiles.size() != b->tiles.size()) {
        PRINTERR("Tile count mismatch: " << a->tiles.size() << " vs " << b->tiles.size());
        return false;
    }
    
    for (size_t i = 0; i < a->tiles.size(); ++i) {
        if (a->tiles[i] != b->tiles[i]) {
            PRINTERR("Tile mismatch at index " << i);
            return false;
        }
    }
    
    if (a->tileColors.size() != b->tileColors.size()) {
        PRINTERR("TileColor count mismatch");
        return false;
    }
    
    const float epsilon = 0.0001f;
    for (size_t i = 0; i < a->tileColors.size(); ++i) {
        const Color& ca = a->tileColors[i];
        const Color& cb = b->tileColors[i];
        
        if (std::abs(ca.r - cb.r) > epsilon || 
            std::abs(ca.g - cb.g) > epsilon ||
            std::abs(ca.b - cb.b) > epsilon || 
            std::abs(ca.a - cb.a) > epsilon) {
            PRINTERR("Color mismatch at index " << i);
            return false;
        }
    }
    
    if (a->entities.size() != b->entities.size()) {
        PRINTERR("Entity count mismatch: " << a->entities.size() << " vs " << b->entities.size());
        return false;
    }
    
    for (size_t i = 0; i < a->entities.size(); ++i) {
        if (!compare_entities(a->entities[i].get(), b->entities[i].get())) {
            PRINTERR("Entity mismatch at index " << i);
            return false;
        }
    }
    
    return true;
}

bool test_direct_serialization() {
    PRINT("\n=== TEST: Direct Serialization ===");
    
    ChunkManager manager;
    
    // Create a chunk manually
    Chunk* original = new Chunk(16, 16, Vector2i(5, 10), nullptr);
    
    // Manually set tiles
    original->tiles = {1, 2, 3, 4, 5};
    original->tileColors = {
        Color(1.0f, 0.0f, 0.0f, 1.0f),
        Color(0.0f, 1.0f, 0.0f, 1.0f),
        Color(0.0f, 0.0f, 1.0f, 1.0f),
        Color(1.0f, 1.0f, 0.0f, 1.0f),
        Color(1.0f, 0.0f, 1.0f, 1.0f)
    };
    
    // Create colonist
    auto colonist = std::make_shared<Colonist>();
    colonist->set_entity_id(12345);
    colonist->set_position(Vector2i(100, 200));
    colonist->set_entity_size(Vector2i(1, 1));
    colonist->set_entity_sprite(42);
    colonist->set_move_speed(5.5f);
    colonist->set_base_move_speed(5.0f);
    colonist->set_active(true);
    colonist->set_must_simulate(true);
    colonist->set_home_coord(Vector2i(50, 60));
    colonist->set_current_job_index(0);
    
    // Add jobs
    EntityJob job1;
    job1.isValid = true;
    job1.complete = false;
    job1.target_coord = Vector2i(150, 250);
    job1.move_algo = "A*";
    job1.priority = 10;
    job1.moveSpeedMultiplier = 1.2f;
    
    EntityJob job2;
    job2.isValid = true;
    job2.complete = true;
    job2.target_coord = Vector2i(75, 125);
    job2.move_algo = "dijkstra";
    job2.priority = 5;
    job2.moveSpeedMultiplier = 0.8f;
    
    std::vector<EntityJob> jobs = {job1, job2};
    colonist->set_job_list(jobs);
    
    original->entities.push_back(colonist);
    
    PRINT("Original entity count: " << original->entities.size());
    PRINT("Original job count: " << colonist->get_job_list().size());
    
    // Serialize
    PackedByteArray data = manager._serialize_chunk(original);
    PRINT("Serialized size: " << data.size() << " bytes");
    
    // Deserialize
    Chunk* deserialized = manager._deserialize_chunk(data);
    
    if (!deserialized) {
        PRINTERR("Deserialization returned null!");
        delete original;
        return false;
    }
    
    PRINT("Deserialized entity count: " << deserialized->entities.size());
    if (deserialized->entities.size() > 0) {
        PRINT("Deserialized job count: " << deserialized->entities[0]->get_job_list().size());
    }
    
    // Compare
    bool result = compare_chunks(original, deserialized);
    
    delete original;
    delete deserialized;
    
    result ? PRINT("PASSED") : PRINT("FAILED");
    return result;
}

int main() {
    PRINT("========================================");
    PRINT("  ChunkManager Serialization Test");
    PRINT("  (Standalone - No Godot)");
    PRINT("========================================");
    
    bool passed = test_direct_serialization();
    
    PRINT("\n========================================");
    PRINT("  Result");
    PRINT("========================================");
    
    if (passed) {
        PRINT("✓ TEST PASSED!");
        return 0;
    } else {
        PRINTERR("✗ TEST FAILED");
        return 1;
    }
}
