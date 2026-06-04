#include "ui.h"
#include <iostream>

// stb_image header - assuming implementation is compiled in stb_image.cpp
#include "stb_image.h"

#ifdef __clang__
// --- SILENCE WARNINGS FOR IMGUI (Apple Clang) ---
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memcall"
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef __clang__
#pragma clang diagnostic pop
// --- END WARNING SILENCE ---
#endif

#include "globals.h"

void GameUI::init(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load game logo
    loadLogoTexture("../assets/logo.png");
}

void GameUI::loadLogoTexture(const char *filename) {
    glGenTextures(1, &logoTexture);
    glBindTexture(GL_TEXTURE_2D, logoTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(false);

    int channels;
    unsigned char *data =
        stbi_load(filename, &logoWidth, &logoHeight, &channels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, logoWidth, logoHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "[GameUI] Failed to load logo texture: " << filename
                  << std::endl;
    }
    stbi_image_free(data);
}

void GameUI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GameUI::render(AppState &currentState, Game &game, GLFWwindow *window) {
    switch (currentState) {
    case AppState::StartScreen:
        drawStartScreen(currentState, window, game);
        break;
    case AppState::Settings:
        drawSettings(currentState, game);
        break;
    case AppState::Paused:
        drawPauseScreen(currentState, window, game);
        break;
    case AppState::GameOver:
        drawGameOver(currentState, game, window);
        break;
    case AppState::InGame:
        drawHUD(game);
        break;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameUI::drawStartScreen(AppState &currentState, GLFWwindow *window,
                             Game &game) {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("StartMenu", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                                              IM_COL32(8, 32, 33, 255));

    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    if (logoTexture != 0) {
        float drawWidth = logoWidth * 0.5f;
        float drawHeight = logoHeight * 0.5f;
        float logoX = (windowWidth - drawWidth) * 0.5f;
        float logoY = windowHeight * 0.15f;

        ImGui::SetCursorPos(ImVec2(logoX, logoY));
        ImGui::Image((void *)(intptr_t)logoTexture,
                     ImVec2(drawWidth, drawHeight));
    }

    const char *welcomeText = "Get ready to brew some trouble!";
    ImVec2 textSize = ImGui::CalcTextSize(welcomeText);
    ImGui::SetCursorPos(
        ImVec2((windowWidth - textSize.x) * 0.5f, windowHeight * 0.50f));
    ImGui::Text("%s", welcomeText);

    float buttonWidth = 250.0f;
    float buttonHeight = 60.0f;
    float buttonStartX = (windowWidth - buttonWidth) * 0.5f;

    ImGui::SetCursorPos(ImVec2(buttonStartX, windowHeight * 0.60f));
    if (ImGui::Button("START GAME", ImVec2(buttonWidth, buttonHeight))) {
        game.restartGame();
        currentState = AppState::InGame;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    ImGui::SetCursorPos(
        ImVec2(buttonStartX, windowHeight * 0.60f + buttonHeight + 20.0f));
    if (ImGui::Button("SETTINGS", ImVec2(buttonWidth, buttonHeight))) {
        currentState = AppState::Settings;
    }

    ImGui::End();
}

void GameUI::drawSettings(AppState &currentState, Game &game) {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                                              IM_COL32(10, 45, 30, 255));

    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    if (logoTexture != 0) {
        float drawWidth = logoWidth * 0.5f;
        float drawHeight = logoHeight * 0.5f;
        float logoX = (windowWidth - drawWidth) * 0.5f;
        float logoY = windowHeight * 0.10f;

        ImGui::SetCursorPos(ImVec2(logoX, logoY));
        ImGui::Image((void *)(intptr_t)logoTexture,
                     ImVec2(drawWidth, drawHeight));
    }

    float buttonWidth = 250.0f;
    float buttonHeight = 50.0f;
    float centerX = (windowWidth - buttonWidth) * 0.5f;

    ImGui::SetCursorPos(ImVec2(centerX, windowHeight * 0.50f));
    if (ImGui::Button("<- BACK TO MENU", ImVec2(buttonWidth, buttonHeight))) {
        currentState = AppState::StartScreen;
    }

    float panelWidth = 550.0f;
    float panelHeight = 300.0f;
    float panelX = (windowWidth - panelWidth) * 0.5f;

    ImGui::SetCursorPos(ImVec2(panelX, windowHeight * 0.60f));
    ImGui::BeginChild("SettingsPanel", ImVec2(panelWidth, panelHeight), true);

    if (ImGui::BeginTabBar("Tabs")) {

        if (ImGui::BeginTabItem("Player")) {
            ImGui::Spacing();
            ImGui::Checkbox("Show Score", &game.settings.displayTime);
            ImGui::Spacing();

            const char *diffs[] = {"Easy", "Medium", "Hard"};
            ImGui::PushItemWidth(200.0f);
            ImGui::Combo("Difficulty", &game.settings.difficultyIdx, diffs, 3);
            ImGui::PopItemWidth();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Basic Controls:");
            ImGui::BulletText("WSAD - Move ship");
            ImGui::BulletText("Right Mouse Button - Move plate");
            ImGui::BulletText("P - Pause/resume game");
            ImGui::BulletText("ESC - Exit game");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Developer")) {
            ImGui::Spacing();
            ImGui::Checkbox("Show Stats (FPS, Draw Calls)",
                            &game.settings.showStats);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Shortcuts (active in game):");
            ImGui::BulletText("H - Toggle Hitboxes");
            ImGui::BulletText("J - Toggle Spawners");
            ImGui::BulletText("R - Instant Scene Reset");

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::EndChild();
    ImGui::End();
}

void GameUI::drawPauseScreen(AppState &currentState, GLFWwindow *window,
                             Game &game) {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("PauseMenu", nullptr, ImGuiWindowFlags_NoDecoration);

    // Półprzezroczyste (Alpha: 200) ciemne cyjanowe tło (widać zatrzymaną grę!)
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                                              IM_COL32(15, 60, 60, 200));

    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    // --- WIELKI NAPIS PAUSE ---
    const char *pauseText = "PAUSE";
    ImGui::SetWindowFontScale(3.0f); // Tymczasowo powiększamy czcionkę
    ImVec2 textSize = ImGui::CalcTextSize(pauseText);
    ImGui::SetCursorPos(
        ImVec2((windowWidth - textSize.x) * 0.5f, windowHeight * 0.35f));
    ImGui::Text("%s", pauseText);
    ImGui::SetWindowFontScale(1.0f); // Wracamy do domyślnej czcionki

    // --- PRZYCISK RESUME ---
    float buttonWidth = 250.0f;
    float buttonHeight = 60.0f;
    float buttonStartX = (windowWidth - buttonWidth) * 0.5f;

    ImGui::SetCursorPos(ImVec2(buttonStartX, windowHeight * 0.50f));
    if (ImGui::Button("RESUME", ImVec2(buttonWidth, buttonHeight))) {
        currentState = AppState::InGame;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        float currentTime = static_cast<float>(glfwGetTime());

        // changing play time
        game.timeBundle.gameStartTime +=
            (currentTime - game.timeBundle.pauseStartTime);

        game.timeBundle.lastFrameTime = currentTime;
    }

    // Opcjonalnie: Powrót do menu
    ImGui::SetCursorPos(
        ImVec2(buttonStartX, windowHeight * 0.50f + buttonHeight + 20.0f));
    if (ImGui::Button("MAIN MENU", ImVec2(buttonWidth, buttonHeight))) {
        currentState = AppState::StartScreen;
    }

    ImGui::End();
}

void GameUI::drawGameOver(AppState &currentState, Game &game,
                          GLFWwindow *window) {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("GameOver", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                                              IM_COL32(60, 10, 10, 255));

    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    if (logoTexture != 0) {
        float drawWidth = logoWidth * 0.5f;
        float drawHeight = logoHeight * 0.5f;
        float logoX = (windowWidth - drawWidth) * 0.5f;
        float logoY = windowHeight * 0.15f;

        ImGui::SetCursorPos(ImVec2(logoX, logoY));
        ImGui::Image((void *)(intptr_t)logoTexture,
                     ImVec2(drawWidth, drawHeight));
    }

    static int finalScore = 0;
    static bool isScoreSaved = false;

    // Jeśli to pierwsza klatka po przegranej, obliczamy i zapisujemy wynik na
    // stałe
    if (!isScoreSaved) {
        finalScore = static_cast<int>(std::floor(game.gameplayTime() * 100));
        isScoreSaved = true;
    }

    // --- POŁĄCZONY TEKST I WYNIK W JEDNEJ LINII ---
    const char *baseText = "Drowned in coffee!  -  ";
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d",
             finalScore); // Używamy ZAMROŻONEGO wyniku

    ImVec2 baseTextSize = ImGui::CalcTextSize(baseText);
    ImVec2 scoreTextSize = ImGui::CalcTextSize(scoreText);

    float totalWidth = baseTextSize.x + scoreTextSize.x;

    ImGui::SetCursorPos(
        ImVec2((windowWidth - totalWidth) * 0.5f, windowHeight * 0.50f));
    ImGui::Text("%s", baseText);

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", scoreText);

    float buttonWidth = 250.0f;
    float buttonHeight = 60.0f;
    float buttonStartX = (windowWidth - buttonWidth) * 0.5f;

    ImGui::SetCursorPos(ImVec2(buttonStartX, windowHeight * 0.60f));
    if (ImGui::Button("PLAY AGAIN", ImVec2(buttonWidth, buttonHeight))) {
        game.restartGame();
        currentState = AppState::InGame;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    ImGui::SetCursorPos(
        ImVec2(buttonStartX, windowHeight * 0.60f + buttonHeight + 20.0f));
    if (ImGui::Button("MAIN MENU", ImVec2(buttonWidth, buttonHeight))) {
        currentState = AppState::StartScreen;
    }

    ImGui::End();
}

void GameUI::drawHUD(Game &game) {
    if (!game.settings.displayTime && !game.settings.showStats) {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImGui::Begin("HUD", nullptr, windowFlags);

    if (game.settings.displayTime) {
        int score = static_cast<int>(std::floor(game.gameplayTime() * 100));
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "SCORE (TIME): %d",
                           score);
    }

    if (game.settings.showStats) {
        if (game.settings.displayTime) {
            ImGui::Separator();
        }

        int fps = std::round(1.0f / (game.deltaTime() + 0.0001f));
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "FPS: %d", fps);
        ImGui::Text("Bullets: %zu", game.bulletBuffer.activeElementCount());
        ImGui::Text("Draw Calls: %zu", gameStateData.drawCallsMade);

        glm::vec3 pPos = game.player->get_pos();
        ImGui::Text("Player Pos: (%.1f, %.1f, %.1f)", pPos.x, pPos.y, pPos.z);
    }

    ImGui::End();
}

void GameUI::shutdown() {
    if (logoTexture != 0) {
        glDeleteTextures(1, &logoTexture);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}