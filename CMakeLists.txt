cmake_minimum_required(VERSION 3.28)

# FIX FOR CMAKE 4.0+: Forces legacy sub-dependencies (like FreeType) 
# to comply with minimum policy rules without breaking the setup.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(gkom)

set(CMAKE_CXX_STANDARD 23)

# Add -g and sanitizers to your compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -g                      # Generate debug information
        -fsanitize=address      # AddressSanitizer
        -fsanitize=undefined    # UndefinedBehaviorSanitizer
        -fno-omit-frame-pointer # Better stack traces
        -Wall
        -Werror
    )
    add_link_options(
        -fsanitize=address 
        -fsanitize=undefined
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
)

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
)