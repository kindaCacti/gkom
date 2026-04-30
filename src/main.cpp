#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <vector>

#include "shaders/utils.h"
#include "shaders/blinn_phong.h"
#include "mesh/mesh_loader.h"
#include "shapes/shape_factory.h"
#include "entities/player.h"
#include "defines.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, Player &p);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static unsigned int createNoiseTexture2D(int width, int height) {
    std::vector<unsigned char> data;
    data.resize(static_cast<std::size_t>(width) *
                static_cast<std::size_t>(height));

    std::mt19937 rng(1337u);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto &b : data) {
        b = static_cast<unsigned char>(dist(rng));
    }

    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, data.data());

    // Make sampling as vec3 work nicely if you ever use it.
    GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("../shaders/blinn-phong.vs", "../shaders/blinn-phong.fs");
    // however you like

    unsigned int tex0 = createNoiseTexture2D(512, 512);

    // Link texture unit 0 to the generic texture sampler.
    ourShader.use();
    ourShader.setInt("tex", 0);

    {
        ShapeFactory sf;
        auto meshOpt = mesh_loader::load_obj("../assets/teapot.obj",
                                             glm::vec3(0.8f, 0.5f, 0.2f));
        sf.registerMesh(meshOpt.value(), "teapot");
        auto teapot = sf.createShape("teapot");
        teapot->scale(glm::vec3(0.4f));
        teapot->translate(glm::vec3(0.f, -1.5f, 0.f));
        Player p(std::move(teapot));
        Player p2(sf.createCube(glm::vec3(0.f, -100.f, 0.f)));
        p2.set_scale(glm::vec3(100.0f, 1.0f, 100.0f));
        p2.set_position(glm::vec3(0.0f, -2.0f, 0.0f));
        BlinnPhongParameters bpp;

        // Player p(sf.createCube(glm::vec3(0.f, 0.f, 0.f)));

        // You can unbind the VAO afterwards so other VAO calls won't
        // accidentally modify this VAO, but this rarely happens. Modifying
        // other VAOs requires a call to glBindVertexArray anyways so we
        // generally don't unbind VAOs (nor VBOs) when it's not directly
        // necessary. glBindVertexArray(0);

        // render loop
        // -----------
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        while (!glfwWindowShouldClose(window)) {
            processInput(window, p);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex0);

            ourShader.use();

            bpp.projection = glm::perspective(
                glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f,
                100.0f);
            bpp.view = glm::lookAt(glm::vec3(0.0f, 10.0f, -10.0f), p.get_pos(),
                                   glm::vec3(0, 1, 0));
            shader_utils::load_blinn_phong_uniforms(ourShader, bpp);
            float rot = static_cast<float>(glfwGetTime());

            p.set_rotation(0.0f, rot);
            p.draw(ourShader);
            p2.draw(ourShader);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteTextures(1, &tex0);
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Player &p) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        p.move(0.f, 0.f, MOVEMENT_SPEED);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        p.move(0.f, 0.f, -MOVEMENT_SPEED);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        p.move(MOVEMENT_SPEED, 0.f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        p.move(-MOVEMENT_SPEED, 0.f);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}
