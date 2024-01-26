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

#include "PointLight.hpp"

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

    std::vector<ENDER::Object *> cubes;

    int i = 0;
    for (auto pos: cubePositions) {
        auto cubeObject = ENDER::Object::createCube(std::string("Cube ")+std::to_string(i));
        cubeObject->setPosition(pos);
        scene->addObject(cubeObject);
        cubes.push_back(cubeObject);
        i++;
    }


    auto *camera = new ENDER::FirstPersonCamera({});

    scene->setCamera(camera);

    auto pointLight = new ENDER::PointLight(pointLightPositions[0], glm::vec3(1));
    scene->addLight(pointLight);


    ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});

    ENDER::Window::addInputCallback([&](int key, ENDER::Window::EventStatus status) {
        if(key == GLFW_KEY_SPACE && status == ENDER::Window::EventStatus::Press) {
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

    float angle = 0;
    while (!ENDER::Window::windowShouldClose()) {
        ENDER::Renderer::begin([&]() {
            for(auto obj: scene->getObjects()) {
                ImGui::Text( obj->getName().c_str());
            }
            for(auto light: scene->getLights()) {
                if(light->type == ENDER::Light::LightType::PointLight)
                    ImGui::Text( "PointLight");
           }
        });

        ENDER::Renderer::shader()->use();





        ENDER::Renderer::shader()->setVec3("viewPos", camera->getPosition());

        ENDER::Renderer::shader()->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ENDER::Renderer::shader()->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        ENDER::Renderer::shader()->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ENDER::Renderer::shader()->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);


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
