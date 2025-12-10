cmake_minimum_required(VERSION 3.20)
project(world_gdextension)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Path to godot-cpp folder
set(GODOT_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp)

# Includes
include_directories(
    ${GODOT_CPP_DIR}/gen/include
    ${GODOT_CPP_DIR}/include
    ${GODOT_CPP_DIR}/include/godot_cpp
    ${GODOT_CPP_DIR}/gdextension
)

# Sources
set(SOURCES
    register_types.cpp
    world.cpp
    chunk.cpp
    perlin_noise.cpp
)


# Output library name
set(LIB_NAME world)
add_library(${LIB_NAME} SHARED ${SOURCES})

# Pick the correct godot-cpp library
if (CMAKE_BUILD_TYPE STREQUAL "editor")
    set(GODOT_CPP_LIB ${GODOT_CPP_DIR}/bin/libgodot-cpp.linux.editor.x86_64.a)
else()
    set(GODOT_CPP_LIB ${GODOT_CPP_DIR}/bin/libgodot-cpp.linux.template_release.x86_64.a)
endif()

target_link_libraries(${LIB_NAME} ${GODOT_CPP_LIB})

# === OPENMP — ADD THESE 6 LINES ONLY ===
find_package(OpenMP REQUIRED)                     # Find OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(${LIB_NAME} OpenMP::OpenMP_CXX)  # Link it
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") # Enable #pragma omp

# Output to the Godot project's bin/ directory
set_target_properties(${LIB_NAME} PROPERTIES
    OUTPUT_NAME "${LIB_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)
