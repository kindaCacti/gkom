#ifndef UI_H
#define UI_H

#include <glad/glad.h> // GLAD must be included before GLFW
#include <GLFW/glfw3.h>
#include "../game.h"

// Enum managing game states
enum class AppState { StartScreen, Settings, InGame, GameOver, Paused };

class GameUI {
  public:
    void init(GLFWwindow *window);
    void newFrame();
    void render(AppState &currentState, Game &game, GLFWwindow *window);
    void shutdown();

  private:
    void drawStartScreen(AppState &currentState, GLFWwindow *window,
                         Game &game);
    void drawSettings(AppState &currentState, Game &game);
    void drawPauseScreen(AppState &currentState, GLFWwindow *window,
                         Game &game); // Ekran Pauzy
    void drawGameOver(AppState &currentState, Game &game, GLFWwindow *window);
    void drawHUD(Game &game);

    // Logo handling
    void loadLogoTexture(const char *filename);
    GLuint logoTexture = 0;
    int logoWidth = 0;
    int logoHeight = 0;
};

#endif // UI_H