#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "world.h"

using namespace godot;

void initialize_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    static bool already_registered = false;
    if (already_registered) {
        UtilityFunctions::print_verbose("World already registered - skipping");
        return;
    }

    UtilityFunctions::print("Registering World class...");

    if (!ClassDB::class_exists("World")) {
        GDREGISTER_CLASS(World);
    } else {
        UtilityFunctions::print_verbose("World class already exists in ClassDB");
    }

    already_registered = true;
}

void uninitialize_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT gdextension_initialize(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization) {

    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_world_module);
    init_obj.register_terminator(uninitialize_world_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
