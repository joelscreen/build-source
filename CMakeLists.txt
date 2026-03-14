cmake_minimum_required(VERSION 3.15)
project(build_alpha VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

# Always build static libs to avoid DLL issues on Windows
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ------------------------------------------------------------
# Dependencies via FetchContent
# ------------------------------------------------------------
include(FetchContent)

# Allow users to override git tags to pin versions when desired
set(R3D_GIT_TAG master CACHE STRING "Git tag/branch for r3d")

# Configure r3d to use the vendored raylib/assimp
set(R3D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(R3D_RAYLIB_VENDORED ON CACHE BOOL "" FORCE)
set(R3D_ASSIMP_VENDORED ON CACHE BOOL "" FORCE)

# r3d requires Python for shader/asset embedding
if(NOT DEFINED PYTHON_EXECUTABLE)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "" FORCE)
endif()

# Fetch r3d
message(STATUS "Downloading and configuring r3d...")
FetchContent_Declare(
    r3d
    GIT_REPOSITORY https://github.com/Bigfoot71/r3d.git
    GIT_TAG ${R3D_GIT_TAG}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(r3d)

# ------------------------------------------------------------
# Application
# ------------------------------------------------------------

# Add source files here
set(SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# r3d doesn't currently expose its public headers via target properties,
# so add its include directory explicitly for the application target.
if(NOT r3d_SOURCE_DIR)
    FetchContent_GetProperties(r3d)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${r3d_SOURCE_DIR}/external/raylib/src
        ${r3d_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        r3d
        raylib
)

# On Windows, also link required system libs
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32 opengl32)
endif()

# ------------------------------------------------------------
# Headers and Libraries
# ------------------------------------------------------------

# Blocks Library
add_library(blocks
    libraries/blocks/blocks.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC blocks)

target_include_directories(blocks
    PUBLIC
        ${r3d_SOURCE_DIR}/external/raylib/src
        ${r3d_SOURCE_DIR}/include
)

target_link_libraries(blocks
    PUBLIC
        r3d
        raylib
)

target_include_directories(${PROJECT_NAME} PUBLIC libraries/blocks)

# Shop menu library
add_library(shop_menu
    libraries/menu/menu.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC shop_menu)

target_include_directories(shop_menu
    PUBLIC
        ${r3d_SOURCE_DIR}/external/raylib/src
        ${r3d_SOURCE_DIR}/include
)

target_link_libraries(shop_menu
    PUBLIC
        r3d
        raylib
)

target_include_directories(${PROJECT_NAME} PUBLIC libraries/menu)
