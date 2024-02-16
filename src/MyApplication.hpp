#pragma once
#include "Point.hpp"
#include <Ender.hpp>
#include <ImGuizmo.h>
#include <Utilities.hpp>
#include <memory>

class MyApplication : public ENDER::Application {
  sptr<ENDER::Framebuffer> viewportFramebuffer;
  sptr<ENDER::Framebuffer> sketchFramebuffer;

  sptr<ENDER::FirstPersonCamera> viewportCamera;
  sptr<ENDER::OrthographicCamera> sketchCamera;

  sptr<ENDER::Shader> lightCubeShader;

  sptr<ENDER::Scene> viewportScene;
  sptr<ENDER::Scene> sketchScene;

  std::vector<sptr<EGEOM::Point>> points;

  sptr<ENDER::Object> selectedObjectViewport;

  ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;

  uint _appWidth;
  uint _appHeight;

  ImVec2 sketchWindowPos; 

  glm::vec3 directionalLightDirection;

  ENDER::DirectionalLight *directionalLight;

public:
  MyApplication(uint appWidth, uint appHeight)
      : ENDER::Application(appWidth, appHeight), _appWidth(appWidth),
        _appHeight(appHeight) {}

  void onStart() override {
    ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});
    ENDER::Utils::applyImguiTheme();

    viewportFramebuffer = ENDER::Framebuffer::create(_appWidth, _appHeight);
    sketchFramebuffer = ENDER::Framebuffer::create(_appWidth, _appHeight);

    lightCubeShader =
        ENDER::Shader::create("../resources/shaders/lightShader.vs",
                              "../resources/shaders/lightShader.fs");

    viewportScene = ENDER::Scene::create();
    sketchScene = ENDER::Scene::create();

    viewportCamera =
        ENDER::FirstPersonCamera::create({0, 5, 0}, {_appWidth, _appHeight});
    sketchCamera =
        ENDER::OrthographicCamera::create({0, 5, 0}, {_appWidth, _appHeight});

    viewportScene->setCamera(viewportCamera);
    sketchScene->setCamera(sketchCamera);

    directionalLightDirection = {-0.2f, -1.0f, -0.3f};

    directionalLight =
        new ENDER::DirectionalLight(directionalLightDirection, {1, 1, 1});
    viewportScene->addLight(directionalLight);

    float u_min = 0;
    float v_min = -glm::pi<float>();

    float u_max = glm::pi<float>();
    float v_max = glm::pi<float>();

    int rows = 50;
    int cols = 50;

    auto sphere = ENDER::Utils::createParametricSurface(
        [](float u, float v) {
          return glm::vec3{glm::sin(u) * glm::cos(v), glm::sin(u) * glm::sin(v),
                           glm::cos(u)};
        },
        u_min, v_min, u_max, v_max, rows, cols);
    viewportScene->addObject(sphere);

    // auto s1 = ENDER::Utils::createParametricSurface(
    //     [](float u, float v) {
    //       return glm::vec3{glm::cos(u), glm::sin(u), v};
    //     },
    //     u_min, v_min, u_max, v_max, rows, cols);
    // s1->setPosition({0, -3, 3});
    // viewportScene->addObject(s1);
    //
    auto cube = ENDER::Object::createCube("cube");
    cube->setPosition({2, 0, 0});
    viewportScene->addObject(cube);

    auto grid = ENDER::Object::createGrid("Grid");

    viewportScene->addObject(grid);
    sketchScene->addObject(grid);
  }

  static float degreeToRadians(float angle) {
    return angle * glm::pi<float>() / 180.0f;
  }
  static float radiansToDegree(float angle) {
    return angle / glm::pi<float>() * 180.0f;
  }

  void handleViewportGUI() {
    ImGui::Begin("Viewport");

    viewportCamera->setActive(ImGui::IsWindowFocused());

    float window_width = ImGui::GetContentRegionAvail().x;
    float window_height = ImGui::GetContentRegionAvail().y;

    viewportCamera->setFramebufferSize({window_width, window_height});

    viewportFramebuffer->rescale(window_width, window_height);

    if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left) &&
        ImGui::IsWindowFocused() && !ImGuizmo::IsUsing()) {
      ImVec2 screen_pos = ImGui::GetCursorScreenPos();
      auto mousePosition = ENDER::Window::getMousePosition();
      auto pickedID = viewportFramebuffer->pickObjAt(
          mousePosition.x - screen_pos.x, (mousePosition.y - screen_pos.y));
      for (auto object : viewportScene->getObjects()) {
        object->setSelected(object->getId() == pickedID);
        if (object->getId() == pickedID)
          selectedObjectViewport = object;
      }
    }

    ImGui::Image((ImTextureID)viewportFramebuffer->getTextureId(),
                 ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    if (selectedObjectViewport) {

      ImGuizmo::SetOrthographic(false);
      ImGuizmo::SetDrawlist();
      ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                        window_width, window_height);

      auto objRotation = selectedObjectViewport->getRotation();

      glm::mat4 model;
      auto rotation = selectedObjectViewport->getRotation();
      rotation = {radiansToDegree(rotation.x), radiansToDegree(rotation.y),
                  radiansToDegree(rotation.z)};
      ImGuizmo::RecomposeMatrixFromComponents(
          glm::value_ptr(selectedObjectViewport->getPosition()),
          glm::value_ptr(rotation),
          glm::value_ptr(selectedObjectViewport->getScale()),
          glm::value_ptr(model));

      auto cameraView = viewportCamera->getView();
      auto cameraProj = viewportCamera->getProjection();

      ImGuizmo::Manipulate(glm::value_ptr(cameraView),
                           glm::value_ptr(cameraProj), currentOperation,
                           ImGuizmo::LOCAL, glm::value_ptr(model));

      if (ImGuizmo::IsUsing()) {

        glm::vec3 newPosition;
        glm::vec3 newRotation;
        glm::vec3 newScale;

        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(model), glm::value_ptr(newPosition),
            glm::value_ptr(newRotation), glm::value_ptr(newScale));
        spdlog::debug("rot: {}, {}, {}", newRotation.x, newRotation.y,
                      newRotation.z);
        spdlog::debug("rot deg: {}, {}, {}", degreeToRadians(newRotation.x),
                      degreeToRadians(newRotation.y),
                      degreeToRadians(newRotation.z));

        selectedObjectViewport->setPosition(newPosition);
        selectedObjectViewport->setRotation({degreeToRadians(newRotation.x),
                                             degreeToRadians(newRotation.y),
                                             degreeToRadians(newRotation.z)});
        selectedObjectViewport->setScale(newScale);
      }
    }

    ImGui::End();
  }

  void handleDebugGUI() {
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
    if (ImGui::SliderFloat3("Direciton",
                            glm::value_ptr(directionalLightDirection), -1, 1)) {
      directionalLight->setDirection(directionalLightDirection);
    }

    for (auto obj : viewportScene->getObjects()) {
      if (obj->selected())
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      ImGui::Text("%s", obj->getName().c_str());
      if (obj->selected())
        ImGui::PopStyleColor();
    }
    for (auto light : viewportScene->getLights()) {
      if (light->type == ENDER::Light::LightType::PointLight)
        ImGui::Text("PointLight");
    }
    ImGui::End();
  }

  void handleSketchGUI() {
    ImGui::Begin("Sketch Editor");

    sketchCamera->setActive(ImGui::IsWindowFocused());

    auto window_width = ImGui::GetContentRegionAvail().x;
    auto window_height = ImGui::GetContentRegionAvail().y;

    sketchCamera->setFramebufferSize({window_width, window_height});

    sketchFramebuffer->rescale(window_width, window_height);
    
    sketchWindowPos = ImGui::GetCursorScreenPos();

    if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left) &&
        ImGui::IsWindowFocused()) {
      auto mousePosition = ENDER::Window::getMousePosition();

      auto mouseScreenPosX = mousePosition.x - sketchWindowPos.x;
      auto mouseScreenPosY = mousePosition.y - sketchWindowPos.y;

      auto pickedID =
          sketchFramebuffer->pickObjAt(mouseScreenPosX, mouseScreenPosY);

      // for (auto object : sketchScene->getObjects()) {
        // object->setSelected(object->getId() == pickedID);
      // }
    }

    ImGui::Image(reinterpret_cast<ImTextureID>(sketchFramebuffer->getTextureId()),
                 ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
  }

  void handleMenuBarGUI() {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Options")) {
        if (ImGui::MenuItem("Close", NULL, false, &_isRunning != NULL))
          close();
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
  }

  void beginDockspace() {
    bool p_open = _isRunning;
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGui::Begin("DockSpace", &p_open, window_flags);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  void endDockspace() { ImGui::End(); }

  void onGUI() override {
    beginDockspace();

    handleMenuBarGUI();
    handleDebugGUI();
    handleViewportGUI();
    handleSketchGUI();

    endDockspace();
  }

  void update(float deltaTime) override {
    viewportCamera->proccessInput();
    ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [&] { close(); });
  }

  void render() override {
    ENDER::Renderer::renderScene(viewportScene, viewportFramebuffer);
    ENDER::Renderer::renderScene(sketchScene, sketchFramebuffer);
  }
  void onKey(int key, ENDER::Window::EventStatus status) override {}

  void onKeyRelease(int key) override {}

  void onMouseClick(ENDER::Window::MouseButton button,
                    ENDER::Window::EventStatus status) override {
    if (button == ENDER::Window::MouseButton::Left &&
        status == ENDER::Window::EventStatus::Press) {
      if (sketchCamera->isActive()) {
        auto mousePosition = ENDER::Window::getMousePosition();

        auto mouseScreenPosX = mousePosition.x - sketchWindowPos.x;
        auto mouseScreenPosY = mousePosition.y - sketchWindowPos.y;

        auto worldPos = sketchCamera->mousePositionToWorldPosition(
            {mouseScreenPosX, mouseScreenPosY});
        auto point = EGEOM::Point::create({worldPos.x, 0, worldPos.y});
        sketchScene->addObject(point);
        points.push_back(point);
      }
    }
  }

  void onClose() {}

  void onMouseMove(uint x, uint y) {}

  void onKeyPress(int key) override {
    switch (key) {
    case GLFW_KEY_SPACE: {
      auto pos = viewportScene->getCamera()->getPosition();

      auto lightCube = ENDER::Object::createCube("Light Debug Cube");
      lightCube->setPosition(pos);
      lightCube->setScale(glm::vec3(0.2f));
      lightCube->setShader(lightCubeShader);
      viewportScene->addObject(lightCube);

      auto pointLight = new ENDER::PointLight(pos, glm::vec3(1));
      viewportScene->addLight(pointLight);
    } break;
    case GLFW_KEY_L: {
      ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Lines);
    } break;
    case GLFW_KEY_N: {
      ENDER::Renderer::setRenderNormals(!ENDER::Renderer::isRenderingNormals());
    } break;
    case GLFW_KEY_K: {
      ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Triangles);
    } break;
    case GLFW_KEY_R: {
      currentOperation = ImGuizmo::OPERATION::ROTATE;
    } break;
    case GLFW_KEY_T: {
      currentOperation = ImGuizmo::OPERATION::TRANSLATE;
    } break;
    case GLFW_KEY_Y: {
      currentOperation = ImGuizmo::OPERATION::SCALE;
    } break;
    }
  }
};
