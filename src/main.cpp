#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <vector>

#include "camera.h"
#include "entities/player.h"
#include "mesh/mesh_loader.h"
#include "shader_s.h"
#include "shapes/shape_factory.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

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
    // Blinn-Phong uniforms
    ourShader.setVec3("lightPos", glm::vec3(2.0f, 2.0f, 2.0f));
    ourShader.setVec3("viewPos", glm::vec3(0.0f, 3.0f, 5.0f));
    ourShader.setVec3("lightColor", glm::vec3(1.0f));
    ourShader.setVec3("baseColor", glm::vec3(0.8f, 0.5f, 0.2f));
    ourShader.setFloat("ambientStrength", 0.15f);
    ourShader.setFloat("specularStrength", 0.5f);
    ourShader.setFloat("shininess", 64.0f);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // float vertices[] = {
    //     // positions         // colors
    //     0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
    //     -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
    //     0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f  // top
    // };

    // unsigned int VBO, VAO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // // bind the Vertex Array Object first, then bind and set vertex
    // buffer(s),
    // // and then configure vertex attributes(s).
    // glBindVertexArray(VAO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
    // GL_STATIC_DRAW);

    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
    //                       (void *)0);
    // glEnableVertexAttribArray(0);
    // // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
    //                       (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    ShapeFactory sf;
    sf.registerMesh("../assets/teapot.obj", "teapot",
                    glm::vec3(0.8f, 0.5f, 0.2f));
    auto teapot = sf.createShape("teapot");
    teapot->transform.scale(glm::vec3(0.4f));
    Player p(std::move(teapot));
    p.transform.translate(glm::vec3(0.f, -1.5f, 0.f));
    Camera cam;
    cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                       static_cast<float>(SCR_HEIGHT));
    cam.setPosition(glm::vec3(0.f, 3.f, 5.f));
    cam.setTarget(glm::vec3(0.f, 0.f, 0.f));

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
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setVec3("viewPos", cam.getPosition());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        p.update(deltaTime);
        ourShader.setMat4("camera", cam.getMatrix());
        p.draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &tex0);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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
