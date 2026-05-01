#ifndef INPUT_H
#define INPUT_H

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

#define KEYBIND_MOVE_FORWARD GLFW_KEY_W
#define KEYBIND_MOVE_BACKWARD GLFW_KEY_S
#define KEYBIND_MOVE_LEFT GLFW_KEY_A
#define KEYBIND_MOVE_RIGHT GLFW_KEY_D
#define KEYBIND_EXIT GLFW_KEY_ESCAPE

struct Game;

void processInput(GLFWwindow *window, Game &game, float deltaTime);

#endif
