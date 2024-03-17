#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera/Camera.hpp"
#include "camera/FlyingCamera.hpp"
#include "shader.hpp"

#include "colors/Color.hpp"
#include "lights/Attenuation.hpp"
#include "lights/DirectionalLight.hpp"
#include "lights/PointLight.hpp"
#include "lights/SpotLight.hpp"

#include "models/Box.hpp"

#include "openGLCommon.hpp"

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;

float lastX = 400, lastY = 300;

std::unique_ptr<Camera> camera =
    std::make_unique<FlyingCamera>(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 movementInput = glm::vec3(0.0f, 0.0f, 0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        movementInput.z += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movementInput.z -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        movementInput.x -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        movementInput.x += 1.0f;
    }

    camera->ProcessKeyboard(movementInput, deltaTime);
}

void mouseCallback([[maybe_unused]] GLFWwindow *window, double xPosition, double yPosition) {
    float xOffset = (xPosition - lastX);
    float yOffset = (yPosition - lastY);
    lastX = xPosition;
    lastY = yPosition;

    camera->ProcessMouseMovement(xOffset, yOffset);
}

void scrollCallback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xOffset, double yOffset) {
    camera->ProcessMouseScroll(yOffset);
}

void framebufferSizeCallback(__attribute__((unused)) GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

struct StbiImageDeleter {
    void operator()(unsigned char *data) { stbi_image_free(data); }
};

GLuint loadTexture(char const *path) {
    GLuint textureId;
    glGenTextures(1, &textureId);

    GLint width, height, nrComponents;

    std::unique_ptr<unsigned char, StbiImageDeleter> data(stbi_load(path, &width, &height, &nrComponents, 0));

    if (!data.get()) {
        std::cout << "Failed to load texture" << std::endl;
        return textureId;
    }

    GLenum format;

    switch (nrComponents) {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.get());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureId;
};

struct GLFWDeleter {
    void operator()(__attribute__((unused)) GLFWwindow *window) { glfwTerminate(); }
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::unique_ptr<GLFWwindow, GLFWDeleter> window(
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Open GL Practice", NULL, NULL)
    );

    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window.get());
    glfwSetFramebufferSizeCallback(window.get(), framebufferSizeCallback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cout << "Failed to load GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window.get(), mouseCallback);
    glfwSetScrollCallback(window.get(), scrollCallback);

    std::filesystem::path root = std::filesystem::current_path();
    std::string shaderFolder = root.string() + "/../shaders/";
    std::string textureFolder = root.string() + "/../textures/";

    GLuint diffuseMap = loadTexture((textureFolder + "container2.png").c_str());
    GLuint specularMap = loadTexture((textureFolder + "container2_specular.png").c_str());

    Light::DirectionalLight directionalLight = Light::DirectionalLight(Color::Red, glm::vec3(-0.2f, -1.0f, -0.3f));

    Light::SpotLight spotLight = Light::SpotLight(
        Color::White,
        Light::BasicAttenuation,
        camera->Position(),
        camera->Front(),
        glm::cos(glm::radians(12.5f)),
        glm::cos(glm::radians(17.5f))
    );

    std::vector<std::pair<glm::vec3, Color::Color>> pointLightDefinitions = {
        std::pair<glm::vec3, Color::Color>(glm::vec3(0.7f, 0.2f, 2.0f), Color::Cyan),
        std::pair<glm::vec3, Color::Color>(glm::vec3(2.3f, -3.3f, -4.0f), Color::Blue),
        std::pair<glm::vec3, Color::Color>(glm::vec3(-4.0f, 2.0f, -12.0f), Color::Purple),
        std::pair<glm::vec3, Color::Color>(glm::vec3(0.0f, 0.0f, -3.0f), Color::Green)
    };

    std::vector<Light::PointLight> pointLights;
    pointLights.reserve(pointLightDefinitions.size());

    for (const std::pair<glm::vec3, Color::Color> &def : pointLightDefinitions) {
        pointLights.push_back(Light::PointLight(def.second, Light::BasicAttenuation, def.first));
    }

    Shader boxShader(
        shaderFolder + "vertex/modelViewProjectionWithNormalAndTex.vert",
        shaderFolder + "fragment/litMaterialTextureMap.frag"
    );

    boxShader.use();
    boxShader.setInt("material.diffuse", 0);
    boxShader.setInt("material.specular", 1);
    boxShader.setDirectionalLight(directionalLight);
    boxShader.setInt("numPointLights", pointLights.size());

    Shader lightShader(shaderFolder + "vertex/modelViewProjection.vert", shaderFolder + "fragment/light.frag");

    std::vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    Box::Init();

    while (!glfwWindowShouldClose(window.get())) {
        const float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processInput(window.get());

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(camera->Zoom(), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera->GetViewMatrix();

        lightShader.use();

        for (const auto &light : pointLights) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, light.position);
            model = glm::scale(model, glm::vec3(0.2f));

            lightShader.setVec3("lightColor", light.color.specular);

            Box::Draw(lightShader, model, view, projection);
        }

        spotLight.position = camera->Position();
        spotLight.direction = camera->Front();

        boxShader.use();
        boxShader.setMat4("view", view);
        boxShader.setMat4("projection", projection);
        boxShader.setFloat("material.shininess", 64.0f);
        boxShader.setFloat("material.glow", sin(currentTime) / 2.0f + 0.5f);
        boxShader.setSpotLight(spotLight);

        for (size_t i = 0; i < pointLights.size(); ++i) {
            boxShader.setPointLight(pointLights[i], i);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        boxShader.setVec3("viewPosition", camera->Position());

        for (size_t i = 0; i < cubePositions.size(); ++i) {
            glm::vec3 cubePosition = cubePositions[i];
            GLfloat angle = 20.0f * i;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePosition);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            Box::Draw(boxShader, model, view, projection);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    Box::Deinit();

    return 0;
}
