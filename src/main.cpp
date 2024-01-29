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

#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "PickingTexture.hpp"

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

int main() {
    std::vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
    };

    int cubesCount = 10;

    for(auto i = 1; i < cubesCount; i++) {
        cubePositions.push_back(glm::vec3(rand()%20, rand()%20, rand()%20));
    }

    ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

    ENDER::Renderer::init();

    spdlog::set_level(spdlog::level::debug);

    auto *lightCubeShader = new ENDER::Shader("../resources/lightShader.vs", "../resources/lightShader.fs");

    auto *scene = new ENDER::Scene();

    std::vector<ENDER::Object *> cubes;

    int i = 0;
    for (auto pos: cubePositions) {
        auto cubeObject = ENDER::Object::createCube(std::string("Cube ") + std::to_string(i));
        cubeObject->setPosition(pos);
        scene->addObject(cubeObject);
        cubes.push_back(cubeObject);
        i++;
    }

    auto *camera = new ENDER::FirstPersonCamera({});

    scene->setCamera(camera);

    ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});

    ENDER::Window::addInputCallback([&](int key, ENDER::Window::EventStatus status) {
        if (key == GLFW_KEY_SPACE && status == ENDER::Window::EventStatus::Press) {
            auto pos = scene->getCamera()->getPosition();

            spdlog::debug("{}, {}, {}", pos.x, pos.y, pos.z);

            auto lightCube = ENDER::Object::createCube("Light Debug Cube");
            lightCube->setPosition(pos);
            lightCube->setScale(glm::vec3(0.2f));
            lightCube->setShader(lightCubeShader);
            scene->addObject(lightCube);

            auto pointLight = new ENDER::PointLight(pos, glm::vec3(1));
            scene->addLight(pointLight);
        }
    });

    glm::vec3 dir = {-0.2f, -1.0f, -0.3f};

    auto directionalLight = new ENDER::DirectionalLight(dir, {1, 1, 1});
    scene->addLight(directionalLight);

    auto grid = ENDER::Object::createGrid("Grid");
    scene->addObject(grid);

    auto *pickingTexture = new ENDER::PickingTexture();
    pickingTexture->init(SCR_WIDTH, SCR_HEIGHT);
    auto *pickingEffect = new ENDER::Shader("../resources/picking.vs", "../resources/picking.fs");

    while (!ENDER::Window::windowShouldClose()) {
        if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
            pickingTexture->enableWriting();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            pickingEffect->use();

            pickingEffect->setInt("gDrawIndex", 0);

            i = 0;
            for (auto object: scene->getObjects()) {
                // Background is zero, the real objects start at 1
                pickingEffect->setInt("gObjectIndex", i + 1);
                ENDER::Renderer::instance().renderObject(object, scene, pickingEffect);
                i++;
            }

            pickingTexture->disableWriting();

            auto mousePosition = ENDER::Window::getMousePosition();
            auto pixel = pickingTexture->readPixel(mousePosition.x, SCR_HEIGHT - mousePosition.y - 1);
            spdlog::debug("{}, {}, {}", pixel.objectID, pixel.primID, pixel.drawID);
            i = 0;
            for (auto object: scene->getObjects()) {
                object->setSelected(pixel.objectID - 1 == i);
                i++;
            }
        }

        ENDER::Renderer::begin([&]() {
            ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
            if (ImGui::SliderFloat3("Direciton", glm::value_ptr(dir), -1, 1)) {
                directionalLight->setDirection(dir);
            }

            for (auto obj: scene->getObjects()) {
                ImGui::Text(obj->getName().c_str());
            }
            for (auto light: scene->getLights()) {
                if (light->type == ENDER::Light::LightType::PointLight)
                    ImGui::Text("PointLight");
            }
        });


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
