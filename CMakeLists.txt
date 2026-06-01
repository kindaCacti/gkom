cmake_minimum_required(VERSION 3.28)

# FIX FOR CMAKE 4.0+: Forces legacy sub-dependencies (like FreeType) 
# to comply with minimum policy rules without breaking the setup.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(gkom)

set(CMAKE_CXX_STANDARD 23)

# Default to Debug if no build type was specified (so existing workflows
# keep getting sanitizers + symbols by default).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # NOTE: -Wall/-Werror are intentionally NOT set globally here; they are
    # applied only to the gkom target below. Otherwise third-party code pulled
    # in via FetchContent (e.g. FreeType) inherits them and fails the build on
    # warnings we don't control (e.g. -Wdangling-pointer in ttgload.c at -O3).

    # Debug: keep -O0, debug info, and sanitizers for catching bugs.
    add_compile_options(
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Debug>:-fsanitize=undefined>
    )
    add_link_options(
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Debug>:-fsanitize=undefined>
    )

    # Release: full optimization, no sanitizers, NDEBUG turns off GLM asserts.
    # -flto=auto lets GCC parallelize the LTRANS stage automatically (otherwise
    # gcc serializes it and emits "using serial compilation of N LTRANS jobs").
    # Clang doesn't accept -flto=auto, so pass plain -flto there.
    add_compile_options(
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-march=native>
        "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto=auto>"
        "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-flto>"
    )
    add_link_options(
        "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto=auto>"
        "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-flto>"
    )

    # RelWithDebInfo: optimized but with symbols, handy for profiling.
    add_compile_options(
        $<$<CONFIG:RelWithDebInfo>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g>
        $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
    )
endif()

find_package(glfw3 REQUIRED)

# -------------------------------------------------------------------
# AUTOMATICALLY FETCH AND BUILD FREETYPE
# -------------------------------------------------------------------
include(FetchContent)

set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
set(FT_DISABLE_ZLIB ON CACHE INTERNAL "")
set(FT_DISABLE_BZIP2 ON CACHE INTERNAL "")
set(FT_DISABLE_PNG ON CACHE INTERNAL "")
set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "")
set(FT_DISABLE_BROTLI ON CACHE INTERNAL "")

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-2
)

FetchContent_MakeAvailable(freetype)
# -------------------------------------------------------------------

# -------------------------------------------------------------------
# AUTOMATICALLY FETCH AND BUILD YAML-CPP
# -------------------------------------------------------------------
# Disable the parts of yaml-cpp we don't need so the build stays small.
set(YAML_CPP_BUILD_TESTS    OFF CACHE INTERNAL "")
set(YAML_CPP_BUILD_TOOLS    OFF CACHE INTERNAL "")
set(YAML_CPP_BUILD_CONTRIB  OFF CACHE INTERNAL "")
set(YAML_CPP_INSTALL        OFF CACHE INTERNAL "")
set(YAML_BUILD_SHARED_LIBS  OFF CACHE INTERNAL "")

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
)

FetchContent_MakeAvailable(yaml-cpp)
# -------------------------------------------------------------------

add_executable(gkom 
    src/main.cpp
    src/game.cpp
    src/input.cpp
    includes/glad/src/glad.c
    includes/stb/stb_image.cpp
    src/entities/player.cpp
    src/entities/bullet.cpp
    src/entities/emiter.cpp
    src/shaders/utils.cpp
    src/globals.cpp
)

# Strict warnings apply only to our own target, not to FetchContent deps.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gkom PRIVATE -Wall -Werror)
endif()

# Define your local headers directly on the executable target (Modern CMake)
target_include_directories(gkom PRIVATE includes/glad/include)
target_include_directories(gkom PRIVATE includes/stb)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE includes/glm)

# Link your dependencies
target_link_libraries(gkom 
    PRIVATE
    glfw 
    ${CMAKE_DL_LIBS} 
    glm
    freetype
    yaml-cpp::yaml-cpp
)