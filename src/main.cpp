#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "game.h"
#include "camera.h"
#include "entities/player.h"
#include "shaders/utils.h"
#include "shaders/shader_params.h"
#include "mesh/mesh_loader.h"
#include "./shaders/shader_s.h"
#include "shapes/shape_factory.h"
#include "defines.h"
#include "input.h"
#include "text/text.h"
#include "state.h"
#include "globals.h"
#include "settings.h"

#include "ui.h"

int main() {
    gameSettings = loadSettingsFromYaml(SETTINGS_FILE_PATH);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window =
        glfwCreateWindow(gameSettings.windowWidth, gameSettings.windowHeight,
                         gameSettings.title.c_str(), NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    {
        Game game;
        glfwSetWindowUserPointer(window, &game);
        glfwSetFramebufferSizeCallback(
            window, [](GLFWwindow *window, int width, int height) {
                Game *game =
                    reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                if (game)
                    game->onFramebufferResize(window, width, height);
            });
        glfwSetCursorPosCallback(
            window, [](GLFWwindow *window, double xpos, double ypos) {
                Game *game =
                    reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                if (game)
                    game->onMouseMove(window, xpos, ypos);
            });

        game.setupScene();
        if (game.loadFont())
            return -1;

        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        game.restartGame();

        // ------------------------------------
        // INICJALIZACJA UI
        // ------------------------------------
        GameUI gameUI;
        gameUI.init(window);

        AppState currentState = AppState::StartScreen;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // GŁÓWNA PĘTLA
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // Ważne: Polling na początku klatki dla ImGui

            // Rozpoczęcie klatki ImGui
            gameUI.newFrame();

            gameStateData.newFrame();
            game.doFramePreprocessing();

            // 1. OBSŁUGA WEJŚCIA
            // Zawsze wywołujemy processInput. Przekazujemy mu currentState,
            // aby samo decydowało czy można się ruszać, czy tylko przełączać pauzę/wyjście.
            processInput(window, game, game.deltaTime(), currentState);

            // 2. AKTUALIZACJA LOGIKI GRY (tylko w trakcie grania)
            if (currentState == AppState::InGame) {
                game.updateScene();

                // Sprawdzanie warunku przegranej
                if (game.shouldEnd()) {
                    currentState = AppState::GameOver;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            // 3. RENDEROWANIE OPENGL (Świat pod spodem)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            game.drawScene(); // Zawsze renderujemy 3D, żeby widzieć tło z menu/pauzą

            // 4. RENDEROWANIE UI (Zawsze na wierzchu)
            gameUI.render(currentState, game, window);

            glfwSwapBuffers(window);
        }

        gameUI.shutdown();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}