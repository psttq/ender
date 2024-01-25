#include <Ender.hpp>

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "FirstPersonCamera.hpp"

#include <imgui.h>

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    glm::vec3 cubePositions[] = {
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

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)
    };

    ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

    ENDER::Renderer::init();

    spdlog::set_level(spdlog::level::debug);

    auto *lightCubeShader = new ENDER::Shader("../resources/lightShader.vs", "../resources/lightShader.fs");

    auto *scene = new ENDER::Scene();

    auto texture = new ENDER::Texture();
    texture->loadFromFile("../resources/textures/container2.png", GL_RGBA);

    std::vector<ENDER::Object *> cubes;

    for (auto pos: cubePositions) {
        auto cubeObject = ENDER::Object::createCube("cube");
        cubeObject->setPosition(pos);
        // cubeObject->setTexture(texture);
        scene->addObject(cubeObject);
        cubes.push_back(cubeObject);
    }

    for(auto pos: pointLightPositions) {
        auto lightCube = ENDER::Object::createCube("cube2");
        lightCube->setPosition(pos);
        lightCube->setScale(glm::vec3(0.2f));
        lightCube->setShader(lightCubeShader);
        scene->addObject(lightCube);
    }

    auto *camera = new ENDER::FirstPersonCamera({});

    scene->setCamera(camera);

    ENDER::Renderer::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});

    float angle = 0;
    while (!ENDER::Window::windowShouldClose()) {
        ENDER::Renderer::begin([]() {
            ImGui::ShowDemoWindow();
        });

        ENDER::Renderer::shader()->use();

        ENDER::Renderer::shader()->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ENDER::Renderer::shader()->setFloat("material.shininess", 64.0f);
        ENDER::Renderer::shader()->setVec3("material.diffuse", {1.0f, 0.5f, 0.31f});
        ENDER::Renderer::shader()->setVec3("material.ambient", {1.0f, 0.5f, 0.31f});


        ENDER::Renderer::shader()->setVec3("viewPos", camera->getPosition());

        ENDER::Renderer::shader()->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ENDER::Renderer::shader()->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ENDER::Renderer::shader()->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        ENDER::Renderer::shader()->setVec3("pointLights[0].position", pointLightPositions[0]);
        ENDER::Renderer::shader()->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ENDER::Renderer::shader()->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[0].constant", 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[0].linear", 0.09f);
        ENDER::Renderer::shader()->setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        ENDER::Renderer::shader()->setVec3("pointLights[1].position", pointLightPositions[1]);
        ENDER::Renderer::shader()->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        ENDER::Renderer::shader()->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[1].constant", 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[1].linear", 0.09f);
        ENDER::Renderer::shader()->setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        ENDER::Renderer::shader()->setVec3("pointLights[2].position", pointLightPositions[2]);
        ENDER::Renderer::shader()->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        ENDER::Renderer::shader()->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[2].constant", 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[2].linear", 0.09f);
        ENDER::Renderer::shader()->setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        ENDER::Renderer::shader()->setVec3("pointLights[3].position", pointLightPositions[3]);
        ENDER::Renderer::shader()->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        ENDER::Renderer::shader()->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[3].constant", 1.0f);
        ENDER::Renderer::shader()->setFloat("pointLights[3].linear", 0.09f);
        ENDER::Renderer::shader()->setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight


        angle += 1.0f * ENDER::Window::deltaTime();

        // cubeObject->setRotation(glm::vec3(0, angle, 0));
        // spdlog::debug(" fps={}", 1 / deltaTime);

        ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });

        ENDER::Renderer::clear();

        ENDER::Renderer::renderScene(scene);

        ENDER::Renderer::end();

        camera->proccessInput();
    }

    delete scene;
    delete camera;
    delete lightCubeShader;

    return 0;
}
