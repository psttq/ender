#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <Ender.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "PickingTexture.hpp"

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

int main() {
  float vertices[] = {
      // positions          // colors           // texture coords
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left

  };
  unsigned int indices[] = {
      0, 1, 3,  // first triangle
      1, 2, 3   // second triangle
  };

  std::vector<glm::vec3> cubePositions = {
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(2.0f, 0.0f, 0.0f),
  };

  int cubesCount = 10;

  for (auto i = 1; i < cubesCount; i++) {
    cubePositions.push_back(glm::vec3(rand() % 20, rand() % 20, rand() % 20));
  }

  ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

  ENDER::Renderer::init();

  spdlog::set_level(spdlog::level::debug);

  auto *lightCubeShader = new ENDER::Shader("../resources/lightShader.vs",
                                            "../resources/lightShader.fs");

  auto *scene = new ENDER::Scene();

  std::vector<ENDER::Object *> cubes;

  int i = 0;
  for (auto pos : cubePositions) {
    auto cubeObject =
        ENDER::Object::createCube(std::string("Cube ") + std::to_string(i));
    cubeObject->setPosition(pos);
    scene->addObject(cubeObject);
    cubes.push_back(cubeObject);
    i++;
  }

  auto *camera = new ENDER::FirstPersonCamera({});

  scene->setCamera(camera);

  ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});

  ENDER::Window::addInputCallback([&](int key,
                                      ENDER::Window::EventStatus status) {
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

  auto step = 0.0f;
  auto speed = 3.0f;

  auto layout = new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3},
                                         {ENDER::LayoutObjectType::Float3},
                                         {ENDER::LayoutObjectType::Float2}});
  auto vbo = new ENDER::VertexBuffer(layout);
  vbo->setData(vertices, sizeof(vertices));

  auto ibo = new ENDER::IndexBuffer(indices, sizeof(indices));

  auto vao = new ENDER::VertexArray();
  vao->addVBO(vbo);
  vao->setIndexBuffer(ibo);

  auto square = new ENDER::Object("square", vao);

  auto debugFramebuffer = new ENDER::Shader("../resources/debugFramebuffer.vs",
                                            "../resources/debugFramebuffer.fs");

  square->setShader(debugFramebuffer);

  while (!ENDER::Window::windowShouldClose()) {
    step += speed * ENDER::Window::deltaTime();

    cubes[0]->setPosition({glm::sin(step), glm::cos(step), 0.5f});

    if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
      auto mousePosition = ENDER::Window::getMousePosition();
      spdlog::debug("Mouse pos: x={}, y={}", mousePosition.x, mousePosition.y);
      auto pickedID =
          ENDER::Renderer::pickObjAt(mousePosition.x, mousePosition.y);
      spdlog::debug("pickedID: {}", pickedID);

      for (auto object : scene->getObjects()) {
        object->setSelected(object->getId() == pickedID);
      }
    }

    ENDER::Renderer::begin([&]() {
      ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
      if (ImGui::SliderFloat3("Direciton", glm::value_ptr(dir), -1, 1)) {
        directionalLight->setDirection(dir);
      }

      ImGui::SliderFloat("Speed", &speed, 0, 100);

      for (auto obj : scene->getObjects()) {
        if (obj->selected())
          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("%s", obj->getName().c_str());
        if (obj->selected()) ImGui::PopStyleColor();
      }
      for (auto light : scene->getLights()) {
        if (light->type == ENDER::Light::LightType::PointLight)
          ImGui::Text("PointLight");
      }
    });

    ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });

    ENDER::Renderer::clear();
    ENDER::Renderer::clearPicking();  // FIXME: maybe unite this with clear()?

    ENDER::Renderer::renderScene(scene);

    debugFramebuffer->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ENDER::Renderer::getPickingTextureID());

    square->getVertexArray()->bind();

    glDrawArrays(GL_TRIANGLES, 0, 36);

    ENDER::Renderer::end();

    camera->proccessInput();
  }

  delete scene;
  delete camera;
  delete lightCubeShader;

  return 0;
}
