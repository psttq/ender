#include <Ender.hpp>
#include <Utilities.hpp>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

int main() {
  spdlog::set_level(spdlog::level::debug);

  float u_max = 2 * glm::pi<float>();
  float v_max = 2 * glm::pi<float>();

  int rows = 25;
  int cols = 25;

  ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

  ENDER::Renderer::init();

  auto *lightCubeShader = new ENDER::Shader("../resources/lightShader.vs",
                                            "../resources/lightShader.fs");

  auto *scene = new ENDER::Scene();

  auto *camera = new ENDER::FirstPersonCamera({});

  scene->setCamera(camera);

  ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});

  ENDER::Window::addInputCallback([&](int key,
                                      ENDER::Window::EventStatus status) {
    if (key == GLFW_KEY_SPACE && status == ENDER::Window::EventStatus::Press) {
      auto pos = scene->getCamera()->getPosition();

      auto lightCube = ENDER::Object::createCube("Light Debug Cube");
      lightCube->setPosition(pos);
      lightCube->setScale(glm::vec3(0.2f));
      lightCube->setShader(lightCubeShader);
      scene->addObject(lightCube);

      auto pointLight = new ENDER::PointLight(pos, glm::vec3(1));
      scene->addLight(pointLight);
    }
    if (key == GLFW_KEY_L && status == ENDER::Window::EventStatus::Press) {
      ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Lines);
    }
    if (key == GLFW_KEY_K && status == ENDER::Window::EventStatus::Press) {
      ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Triangles);
    }
  });

  glm::vec3 dir = {-0.2f, -1.0f, -0.3f};

  auto directionalLight = new ENDER::DirectionalLight(dir, {1, 1, 1});
  scene->addLight(directionalLight);

  auto grid = ENDER::Object::createGrid("Grid");
  // scene->addObject(grid);

  auto texture = new ENDER::Texture();
  texture->loadFromFile("../resources/textures/container.jpg", GL_RGB);

  auto sphere = ENDER::Utils::createParametricSurface(
      [](float u, float v) {
        return glm::vec3{glm::sin(u) * glm::cos(v), glm::sin(u) * glm::sin(v),
                         glm::cos(u)};
      },
      u_max, v_max, rows, cols);
  scene->addObject(sphere);

  while (!ENDER::Window::windowShouldClose()) {
    if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
      auto mousePosition = ENDER::Window::getMousePosition();
      auto pickedID =
          ENDER::Renderer::pickObjAt(mousePosition.x, mousePosition.y);

      for (auto object : scene->getObjects()) {
        object->setSelected(object->getId() == pickedID);
      }
    }

    ENDER::Renderer::begin([&]() {
      ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
      if (ImGui::SliderFloat3("Direciton", glm::value_ptr(dir), -1, 1)) {
        directionalLight->setDirection(dir);
      }

      for (auto obj : scene->getObjects()) {
        if (obj->selected())
          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("%s", obj->getName().c_str());
        if (obj->selected())
          ImGui::PopStyleColor();
      }
      for (auto light : scene->getLights()) {
        if (light->type == ENDER::Light::LightType::PointLight)
          ImGui::Text("PointLight");
      }
    });

    ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });

    ENDER::Renderer::clear();
    ENDER::Renderer::clearPicking(); // FIXME: maybe unite this with clear()?

    ENDER::Renderer::renderScene(scene);

    ENDER::Renderer::end();

    camera->proccessInput();
  }

  delete scene;
  delete camera;
  delete lightCubeShader;

  return 0;
}
