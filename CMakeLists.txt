cmake_minimum_required(VERSION 3.28)
project(gkom)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 REQUIRED)

include_directories(
    includes/glad/include
)


add_executable(gkom 
    src/main.cpp
    includes/glad/src/glad.c
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE includes/glm)

target_link_libraries(gkom 
    glfw 
    ${CMAKE_DL_LIBS} 
    glm
)