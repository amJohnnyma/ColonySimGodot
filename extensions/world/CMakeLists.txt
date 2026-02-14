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

# ============================================================================
# === TEST EXECUTABLE ===
# ============================================================================

# Configure and build
# Clean and rebuild with full debug info
#cd build
#rm -rf *
#cmake .. -DCMAKE_BUILD_TYPE=Debug
#cmake --build . --target test_serialization

# Run with GDB
#gdb ../../../bin/test_serialization

#In GDB:
#(gdb) run
# Wait for crash...
#(gdb) bt full
# Shows full backtrace with variable values

# Valgrind
# valgrind --leak-check=full --track-origins=yes ../../../bin/test_serialization

option(BUILD_TESTS "Build test executable for serialization" ON)

if(BUILD_TESTS)
    # Test sources - only the files needed for serialization testing
    set(TEST_SOURCES
        entity.cpp
        colonist.cpp
        building.cpp
        item.cpp
        entityDataReader.cpp
        terrainGenerator.cpp
        perlin_noise.cpp
        world.cpp
        chunk.cpp
        chunk_manager.cpp
        test_chunk_serialization.cpp  # The test file
    )
    
    # Create test executable
    add_executable(test_serialization ${TEST_SOURCES})
    
    # Same includes as main library
    target_include_directories(test_serialization
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}"
            SYSTEM "${GODOT_CPP_DIR}/include"
            SYSTEM "${GODOT_CPP_DIR}/gen/include"
            SYSTEM "${GODOT_CPP_DIR}/gdextension"
    )
    
    # Link against godot-cpp
    target_link_libraries(test_serialization
        PRIVATE
            "${GODOT_CPP_LIB}"
            Threads::Threads
    )
    
    # Add OpenMP
    if(OpenMP_CXX_FOUND)
        target_link_libraries(test_serialization PRIVATE OpenMP::OpenMP_CXX)
        target_compile_options(test_serialization PRIVATE ${OpenMP_CXX_FLAGS})
    endif()
    
    # Output to bin directory
    set_target_properties(test_serialization PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
    )
    
    # Enable CTest
    enable_testing()
    
    # Add test
    add_test(
        NAME ChunkSerializationTest
        COMMAND test_serialization
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    
    # Custom target to build and run tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_serialization
        COMMENT "Running serialization tests..."
    )
    
    message(STATUS "Tests enabled. Use 'make test_serialization' to build, 'make test' or 'make run_tests' to run.")
endif()
