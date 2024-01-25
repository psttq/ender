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

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

    ENDER::Renderer::init();

    spdlog::set_level(spdlog::level::debug);

    auto *scene = new ENDER::Scene();

    auto texture1 = new ENDER::Texture();
    texture1->loadFromFile("../resources/textures/container.jpg", GL_RGB);

    auto texture2 = new ENDER::Texture();
    texture2->loadFromFile("../resources/textures/awesomeface.png", GL_RGBA);

    auto cubeObject = ENDER::Object::createCube("cube");
    cubeObject->setTexture(texture1);
    cubeObject->setPosition(glm::vec3(1.5f, 0.2f, -1.5f));

    auto cubeObject2 = ENDER::Object::createCube("cube2");
    cubeObject2->setTexture(texture2);
    cubeObject2->setPosition(glm::vec3(-1.5f, 0.2f, -1.5f));

    scene->addObject(cubeObject);
    scene->addObject(cubeObject2);

    auto *camera = new ENDER::FirstPersonCamera({});

    scene->setCamera(camera);

    ENDER::Renderer::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});

    ENDER::Window::disableCursor();

    float angle = 0;
    while (!ENDER::Window::windowShouldClose()) {
        angle += 1.0f * ENDER::Window::deltaTime();
        cubeObject->setRotation(glm::vec3(0, angle, 0));
        // spdlog::debug(" fps={}", 1 / deltaTime);

        ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });


        ENDER::Renderer::clear();

        ENDER::Renderer::renderScene(scene);

        ENDER::Renderer::swapBuffers();
        ENDER::Window::flash();

        camera->proccessInput();
    }

    delete cubeObject;
    delete cubeObject2;
    delete scene;
    delete camera;

    return 0;
}
