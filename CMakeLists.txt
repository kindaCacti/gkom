cmake_minimum_required(VERSION 3.28)
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

include_directories(includes/glad/include)

add_executable(gkom 
    src/main.cpp
    src/game.cpp
    src/input.cpp
    includes/glad/src/glad.c
    src/entities/player.cpp
    src/entities/bullet.cpp
    src/entities/emiter.cpp
    src/shaders/utils.cpp
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE includes/glm)

target_link_libraries(gkom 
    glfw 
    ${CMAKE_DL_LIBS} 
    glm
)