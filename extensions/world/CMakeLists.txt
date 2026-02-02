cmake_minimum_required(VERSION 3.20)
project(world_gdextension LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Path to godot-cpp folder
set(GODOT_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp")

# === Sources ===
set(SOURCES
    register_types.cpp
    entity.cpp
    colonist.cpp
    building.cpp
    item.cpp
    entityDataReader.cpp
    world.cpp
    chunk.cpp
    perlin_noise.cpp
    terrainGenerator.cpp
    chunk_manager.cpp
)

# === Create the library target FIRST ===
add_library(world SHARED ${SOURCES})

# === Now it's safe to set properties on the target ===
target_include_directories(world
    PRIVATE
        # Your own project includes (warnings WANTED here)
        "${CMAKE_CURRENT_SOURCE_DIR}"          # ← add this if you have headers in the root or subfolders
        # "${CMAKE_CURRENT_SOURCE_DIR}/src"   # ← if you use a src/ folder

        # godot-cpp headers → treat as SYSTEM = suppress most warnings from inside them
        SYSTEM "${GODOT_CPP_DIR}/include"
        SYSTEM "${GODOT_CPP_DIR}/gen/include"
        SYSTEM "${GODOT_CPP_DIR}/gdextension"
)

# Pick the correct godot-cpp static library based on build type / variant
if (CMAKE_BUILD_TYPE STREQUAL "editor" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GODOT_CPP_LIB "${GODOT_CPP_DIR}/bin/libgodot-cpp.linux.editor.x86_64.a")
else()
    set(GODOT_CPP_LIB "${GODOT_CPP_DIR}/bin/libgodot-cpp.linux.template_release.x86_64.a")
endif()

# Linking
target_link_libraries(world
    PRIVATE
        "${GODOT_CPP_LIB}"
)

# === OpenMP ===
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(world PRIVATE OpenMP::OpenMP_CXX)
    target_compile_options(world PRIVATE ${OpenMP_CXX_FLAGS})
endif()

# === Threads (for std::thread etc.) ===
find_package(Threads REQUIRED)
target_link_libraries(world PRIVATE Threads::Threads)

# Optional: more flags (warnings, optimization)
target_compile_options(world PRIVATE) #-Wall -Wextra
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(world PRIVATE -O3)
endif()

# Output settings
set_target_properties(world PROPERTIES
    OUTPUT_NAME "world"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
    PREFIX "lib"          # → libworld.so
    SUFFIX ".so"
)
