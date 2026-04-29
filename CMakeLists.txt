cmake_minimum_required(VERSION 3.28)
project(gkom)

set(CMAKE_CXX_STANDARD 23)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

find_package(glfw3 REQUIRED)

include_directories(
    includes/glad/include
)


add_executable(gkom 
    src/main.cpp
    includes/glad/src/glad.c
    src/shapes/shape.h
    src/entities/player.cpp
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE includes/glm)

target_link_libraries(gkom 
    glfw 
    ${CMAKE_DL_LIBS} 
    glm
)