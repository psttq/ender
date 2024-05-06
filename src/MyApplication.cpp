#include "MyApplication.hpp"
#include "BufferLayout.hpp"
#include "IconsFontAwesome5.h"
#include "ImGuizmo.h"
#include "Point.hpp"
#include "Renderer.hpp"
#include "Sketch.hpp"
#include "Spline1.hpp"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <PivotPlane.hpp>
#include <Utilities.hpp>
#include <fstream>
#include <glm/gtx/rotate_vector.hpp>
#include <iterator>
#include <memory>
#include <toml.hpp>
MyApplication::MyApplication(uint appWidth, uint appHeight)
    : ENDER::Application(appWidth, appHeight), _appWidth(appWidth),
      _appHeight(appHeight) {}

void MyApplication::onStart() {
  ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});
  ENDER::Utils::applyImguiTheme();

  ImGuiIO &io = ImGui::GetIO();

  ImFontConfig config;
  config.MergeMode = true;
  //    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon
  //    monospaced
  static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
  io.Fonts->AddFontFromFileTTF("../resources/fa-solid-900.ttf", 13.0f, &config,
                               icon_ranges);

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
      ENDER::OrthographicCamera::create({-2, 5, 1.5f}, {_appWidth, _appHeight});

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

  int rows = 300;
  int cols = 300;

  sphere = ENDER::Utils::createParametricSurface(
      [](float u, float v) {
        return glm::vec3{glm::sin(u) * glm::cos(v), glm::sin(u) * glm::sin(v),
                         glm::cos(u)};
      },
      u_min, v_min, u_max, v_max, rows, cols);

  viewportScene->addObject(sphere);
  sphere->isSelectable = true;

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
  cube->isSelectable = true;
  viewportScene->addObject(cube);

  auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);

  auto newSketch = EGEOM::Sketch::create(
      "Sketch " + std::to_string(sketches.size()), spline);
  sketches.push_back(newSketch);
  currentSketchId = sketches.size() - 1;

  auto grid = ENDER::Object::createGrid("Grid");

  viewportScene->addObject(grid);
  sketchScene->addObject(grid);

}

void MyApplication::handleOperationPropertiesGUI() {
  if (currentTool == Tools::Extrude) {
    ImGui::Begin("Extrude");

    if (ImGui::DragFloat3("Extrude Direction", glm::value_ptr(extrudeDirection),
                          0.1f, 0, 1)) {
    }

    ImGui::DragFloat("Extrude Height", &extrudeHeight, 0.1);

    ImGui::End();
  }
  else if (currentTool == Tools::Rotate) {
      ImGui::Begin("Rotate");
      ImGui::DragFloat("Rotate radius", &rotateRadius, 0.1);
      ImGui::DragFloat("Rotate angle", &rotateAngle, 0.1, 0, glm::pi<float>()*2);
      ImGui::End();
  }
}

void MyApplication::handleViewportGUI() {
  ImGui::Begin("Viewport");

  viewportCamera->setActive(ImGui::IsWindowFocused());

  float window_width = ImGui::GetContentRegionAvail().x;
  float window_height = ImGui::GetContentRegionAvail().y;

  viewportCamera->setFramebufferSize({window_width, window_height});

  viewportFramebuffer->rescale(window_width, window_height);
  if (ImGui::IsWindowFocused()) {
    activeWindow = Windows::Viewport;
  }

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

  ImGui::Image(
      reinterpret_cast<ImTextureID>(viewportFramebuffer->getTextureId()),
      ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
  //    if (ImGui::BeginPopupContextItem("popup")) // <-- use last item id as
  //    popup id
  //    {
  //        if (ImGui::Button("Create Pivot Plane")) {
  //
  //        }
  //        if (ImGui::Button("Close"))
  //            ImGui::CloseCurrentPopup();
  //        ImGui::EndPopup();
  //    }

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

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                         currentOperation, ImGuizmo::LOCAL,
                         glm::value_ptr(model));

    if (currentTool == Tools::Extrude &&
        selectedObjectViewport->label == "PivotPlane") {
      glm::vec3 p1 = selectedObjectViewport->getPosition();
      auto p2 = p1 + extrudeDirection * extrudeHeight;

      auto diff = p2 - p1;
      glm::vec3 xNorm(1.0, 0.0f, 0.0);
      glm::vec3 yNorm(0.0, 1.0f, 0.0);
      glm::vec3 zNorm(0.0, 0.0f, 1.0);

      auto _rotation = selectedObjectViewport->getRotation();

      diff = glm::rotate(diff, _rotation.x, xNorm); // Rotate on X axis
      // yNorm = glm::rotate(yNorm, -_rotation.x, xNorm);
      // zNorm = glm::rotate(zNorm, -_rotation.x, xNorm);
      diff = glm::rotate(diff, _rotation.y, yNorm); // Rotate on Y axis
      // zNorm = glm::rotate(zNorm, -_rotation.y, yNorm);
      diff = glm::rotate(diff, _rotation.z, zNorm); // Rotate

      p2 = p1 + diff;

      ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                          0xFF110055);
    }
    else if (currentTool == Tools::Rotate &&
              selectedObjectViewport->label == "PivotPlane") {
        glm::vec3 p1 = selectedObjectViewport->getPosition();
        p1+=glm::vec3{1,0,0}* rotateRadius;
        auto p2 = p1 + glm::vec3{0, 0, 1};

        auto diff = p2 - p1;
        glm::vec3 xNorm(1.0, 0.0f, 0.0);
        glm::vec3 yNorm(0.0, 1.0f, 0.0);
        glm::vec3 zNorm(0.0, 0.0f, 1.0);

        auto _rotation = selectedObjectViewport->getRotation();

        diff = glm::rotate(diff, _rotation.x, xNorm); // Rotate on X axis
        // yNorm = glm::rotate(yNorm, -_rotation.x, xNorm);
        // zNorm = glm::rotate(zNorm, -_rotation.x, xNorm);
        diff = glm::rotate(diff, _rotation.y, yNorm); // Rotate on Y axis
        // zNorm = glm::rotate(zNorm, -_rotation.y, yNorm);
        diff = glm::rotate(diff, _rotation.z, zNorm); // Rotate

        p2 = p1 + diff;

        ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                            0xFF110055);
    }

    if (ImGuizmo::IsUsing()) {

      glm::vec3 newPosition;
      glm::vec3 newRotation;
      glm::vec3 newScale;

      ImGuizmo::DecomposeMatrixToComponents(
          glm::value_ptr(model), glm::value_ptr(newPosition),
          glm::value_ptr(newRotation), glm::value_ptr(newScale));

      spdlog::info("{} {} {}", newRotation.x, newRotation.y, newRotation.z);

      selectedObjectViewport->setPosition(newPosition);

      selectedObjectViewport->setRotation({degreeToRadians(newRotation.x),
                                           degreeToRadians(newRotation.y),
                                           degreeToRadians(newRotation.z)});
      selectedObjectViewport->setScale(newScale);
    }
  }

  ImGui::End();
}

void MyApplication::handleDebugGUI() {
  ImGui::Begin("Debug");
  ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());

  if (ImGui::SliderInt("Interpolation Points Count", &interpolationPointsCount,
                       2, 300)) {
    sketches[currentSketchId]->getSpline()->setInterpolationPointsCount(
        interpolationPointsCount);
    sketches[currentSketchId]->getSpline()->update();
  }
  ImGui::Checkbox("Render Spline Points", &renderDebugSplinePoints);
  sphere->material.drawImguiEdit();
  if (ImGui::SliderFloat3("Direction",
                          glm::value_ptr(directionalLightDirection), -1, 1)) {
    directionalLight->setDirection(directionalLightDirection);
  }
  glm::vec3 rot;
  if (selectedObjectViewport)
    rot = selectedObjectViewport->getRotation();
  else
    rot = {};
  if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -4, 4)) {
    if (selectedObjectViewport)
      selectedObjectViewport->setRotation(rot);
  }

  for (auto obj : viewportScene->getObjects()) {
    if (obj->selected())
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    ImGui::Text("%s", obj->getName().c_str());
    if (obj->selected())
      ImGui::PopStyleColor();
  }
  for (auto light : viewportScene->getLights()) {
#include <memory>
    if (light->type == ENDER::Light::LightType::PointLight)
      ImGui::Text("PointLight");
  }
  ImGui::End();
}

void MyApplication::handleSketchGUI() {
  ImGui::Begin("Sketch Editor");

  sketchCamera->setActive(ImGui::IsWindowFocused());

  if (ImGui::IsWindowFocused()) {
    activeWindow = Windows::SketchEditor;
  }

  auto window_width = ImGui::GetContentRegionAvail().x;
  auto window_height = ImGui::GetContentRegionAvail().y;

  sketchCamera->setFramebufferSize({window_width, window_height});

  sketchFramebuffer->rescale(window_width, window_height);

  sketchWindowPos = ImGui::GetCursorScreenPos();

  ImGui::Image(reinterpret_cast<ImTextureID>(sketchFramebuffer->getTextureId()),
               ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

  ImGui::End();
}

void MyApplication::handleMenuBarGUI() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      if (ImGui::MenuItem("Close", NULL, false, &_isRunning != NULL))
        close();
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

void MyApplication::beginDockspace() {
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
  ImGui::ShowDemoWindow();
}

void MyApplication::endDockspace() { ImGui::End(); }

void MyApplication::handleToolbarGUI() {
  ImGui::Begin("Toolbar", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
  bool setStyle = false;
  if (currentTool == Tools::Cursor) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
    setStyle = true;
  }
  if (ImGui::Button(ICON_FA_MOUSE_POINTER)) {
    currentTool = Tools::Cursor;
  }
  if (setStyle) {
    ImGui::PopStyleColor(3);
    setStyle = false;
  }
  ImGui::SameLine();
  if (currentTool == Tools::Pencil) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
    setStyle = true;
  }
  if (ImGui::Button(ICON_FA_PENCIL_ALT)) {
    currentTool = Tools::Pencil;
  }
  ImGui::SetItemTooltip("Sketch Edit Tool");

  if (setStyle) {
    ImGui::PopStyleColor(3);
    setStyle = false;
  }

  if (ImGui::Button(ICON_FA_VECTOR_SQUARE)) {
    createPivotPlane();
  }
  ImGui::SetItemTooltip("Create Pivot Plane");
  ImGui::SameLine();
  if (currentTool == Tools::Extrude) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
    setStyle = true;
  }
  if (ImGui::Button(ICON_FA_ARROWS_ALT_V)) {
    currentTool = Tools::Extrude;
  }
  ImGui::SetItemTooltip("Extrude Tool");

  if (setStyle) {
    ImGui::PopStyleColor(3);
    setStyle = false;
  }
    ImGui::SameLine();
    if (currentTool == Tools::Rotate) {
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
        setStyle = true;
    }
    if (ImGui::Button(ICON_FA_SYNC)) {
        currentTool = Tools::Rotate;
    }
    ImGui::SetItemTooltip("Rotate Tool");

    if (setStyle) {
        ImGui::PopStyleColor(3);
        setStyle = false;
    }

  ImGui::End();
}

void MyApplication::onGUI() {
  beginDockspace();

  handleMenuBarGUI();
  handleDebugGUI();
  handleViewportGUI();
  handleSketchGUI();
  handleToolbarGUI();
  handleSketchSideGUI();
  handleObjectsGUI();
  handlePropertiesGUI();
  handleOperationPropertiesGUI();

  endDockspace();
}

void MyApplication::update(float deltaTime) { viewportCamera->proccessInput(); }

void MyApplication::render() {
  ENDER::Renderer::renderScene(viewportScene, viewportFramebuffer);
  ENDER::Renderer::renderScene(sketchScene, sketchFramebuffer);

  if (currentSketchId == -1)
    return;

  if (renderDebugSplinePoints)
    for (auto p :
         sketches[currentSketchId]->getSpline()->getInterpolatedPoints()) {
      p->material.ambient = {0.0, 0.6, 0.6};
      p->material.diffuse = {0.0, 0.6, 0.6};

      ENDER::Renderer::renderObject(p, sketchScene, sketchFramebuffer);
    }

  for (auto p : sketches[currentSketchId]->getSpline()->getPoints()) {
    ENDER::Renderer::renderObject(p, sketchScene, sketchFramebuffer);
  }
  ENDER::Renderer::renderObject(sketches[currentSketchId]->getSpline(),
                                sketchScene, sketchFramebuffer);
}

void MyApplication::onKey(int key, ENDER::Window::EventStatus status) {}

void MyApplication::onKeyRelease(int key) {}

void MyApplication::onMouseClick(ENDER::Window::MouseButton button,
                                 ENDER::Window::EventStatus status) {
  if (button == ENDER::Window::MouseButton::Left &&
      status == ENDER::Window::EventStatus::Release) {
    mouseMove = false;
    selectedObjectSketch = nullptr;
  }
  if (button == ENDER::Window::MouseButton::Left &&
      status == ENDER::Window::EventStatus::Press) {
    if (sketchCamera->isActive()) {
      if (currentSketchId == -1)
        return;
      mouseMove = true;
      auto mousePosition = ENDER::Window::getMousePosition();

      auto mouseScreenPosX = mousePosition.x - sketchWindowPos.x;
      auto mouseScreenPosY = mousePosition.y - sketchWindowPos.y;

      if (mouseScreenPosX < 0 || mouseScreenPosY < 0)
        return;

      sptr<ENDER::Object> currentSelected;
      auto pickedID =
          sketchFramebuffer->pickObjAt(mouseScreenPosX, mouseScreenPosY);
      for (auto object : sketches[currentSketchId]->getSpline()->getPoints()) {
        object->setSelected(object->getId() == pickedID);
        if (object->getId() == pickedID) {
          currentSelected = object;
        }
      }
      if (currentTool == Tools::Pencil) {
        sptr<EGEOM::Point> point;
        if (!currentSelected) {
          auto worldPos = sketchCamera->mousePositionToWorldPosition(
              {mouseScreenPosX, mouseScreenPosY});
          point = EGEOM::Point::create({worldPos.x, 0, worldPos.y});
        } else {
          point = std::static_pointer_cast<EGEOM::Point>(currentSelected);
        }
        point->isSelectable = true;
        sketches[currentSketchId]->getSpline()->addPoint(point);

      } else if (currentTool == Tools::Cursor) {
        if (currentSelected) {
          selectedObjectSketch = currentSelected;
          justSelected = true;
          for (auto object :
               sketches[currentSketchId]->getSpline()->getPoints())
            object->setSelected(false);
          currentSelected->setSelected(true);
        }
      }
    }
  }
}

void MyApplication::onKeyPress(int key) {
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
  case GLFW_KEY_U: {
    if (sketches.size() == 0)
      return;
    if(currentTool == Tools::Extrude) {
        auto obj = ENDER::Utils::createParametricSurface(
                [&](float u, float v) {
                    auto point = sketches[currentSketchId]->getSpline()->getSplinePoint(u) +
                                 v * extrudeHeight * extrudeDirection;
                    return point;
                },
                0, 0, 1, 1, 200, 200);

        obj->setPosition(selectedObjectViewport->getPosition());
        obj->setRotation(selectedObjectViewport->getRotation());

        viewportScene->addObject(obj);
        obj->isSelectable = true;
    }
    else if(currentTool == Tools::Rotate) {
        auto obj = ENDER::Utils::createParametricSurface(
                [&](float u, float v) {
                    auto splinePoint =
                            sketches[currentSketchId]->getSpline()->getSplinePoint(u) + glm::vec3{-rotateRadius, 0, 0};
                    auto point = glm::vec3{rotateRadius, 0, 0} +
                                 glm::vec3{splinePoint.x * glm::cos(v), splinePoint.x * glm::sin(v), splinePoint.z};
                    return point;
                },
                0, 0, 1, rotateAngle, 200, 200);

        obj->setPosition(selectedObjectViewport->getPosition());
        obj->setRotation(selectedObjectViewport->getRotation());

        viewportScene->addObject(obj);
        obj->isSelectable = true;
    }
  } break;
  }
}

void MyApplication::onClose() {}

void MyApplication::onMouseMove(uint x, uint y) {
  if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
    if (currentTool == Tools::Cursor && selectedObjectSketch != nullptr &&
        sketchCamera->isActive() && mouseMove && currentSketchId != -1) {
      auto mousePosition = ENDER::Window::getMousePosition();

      auto mouseScreenPosX = mousePosition.x - sketchWindowPos.x;
      auto mouseScreenPosY = mousePosition.y - sketchWindowPos.y;

      if (mouseScreenPosX < 0 || mouseScreenPosY < 0)
        return;

      auto worldPos = sketchCamera->mousePositionToWorldPosition(
          {mouseScreenPosX, mouseScreenPosY});

      selectedObjectSketch->setPosition({worldPos.x, 0, worldPos.y});
      sketches[currentSketchId]->getSpline()->update();
    }
  }
}

void MyApplication::handleSketchSideGUI() {
  ImGui::Begin("Sketches");
  if (ImGui::Button("Add sketch")) {
    auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);

    auto newSketch = EGEOM::Sketch::create(
        "Sketch " + std::to_string(sketches.size()), spline);
    sketches.push_back(newSketch);
    currentSketchId = sketches.size() - 1;
  }

  if (ImGui::Button("Save sketch")) {
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("CSF", "Choose File", ".toml");
  }

  if (ImGuiFileDialog::Instance()->Display("CSF")) {
    if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

      toml::table tbl;

      auto currentSketch = sketches[currentSketchId];
      auto spline = currentSketch->getSpline();
      toml::array points;
      for (auto &p : spline->getPoints()) {
        auto position = p->getPosition();
        auto pointTable = toml::table{
            {"x", position.x}, {"y", position.y}, {"z", position.z}};

        points.push_back(pointTable);
      }

      switch (spline->getSplineType()) {
      case EGEOM::Spline1::SplineType::LinearInterpolation: {

        tbl = toml::table{{"type", static_cast<int>(spline->getSplineType())},
                          {"points", points}};
      } break;
      case EGEOM::Spline1::SplineType::NURBS: {
        auto splineBuilder =
            spline->getSplineBuilder<EGEOM::RationalBSplineBuilder>();
        auto knotVector = splineBuilder.knotVector;
        auto weights = splineBuilder.weights;
        auto power = splineBuilder.bSplinePower;

        auto knotVectorToml = toml::array{};
        for (auto &e : knotVector) {
          knotVectorToml.push_back(e);
        }

        auto weigtsToml = toml::array{};
        for (auto &e : weights) {
          weigtsToml.push_back(e);
        }

        tbl = toml::table{{"type", static_cast<int>(spline->getSplineType())},
                          {"power", power},
                          {"knotVector", knotVectorToml},
                          {"weights", weigtsToml},
                          {"points", points}};

      } break;
      default: {
        spdlog::error("Can't write sketch of this type!");
      }
      }

      if (!tbl.empty()) {
        std::ofstream fs(filePathName);
        fs << tbl;
        spdlog::info("Sketch successefully written to {}", filePathName);
      }
      // action
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }

  ImGui::SameLine();

  if (ImGui::Button("Load sketch")) {
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("CLF", "Choose File", ".toml");
  }

  if (ImGuiFileDialog::Instance()->Display("CLF")) {
    if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

      auto tbl = toml::parse_file(filePathName);

      auto splineTypeId = *tbl["type"].value<int>();

      auto splineType = static_cast<EGEOM::Spline1::SplineType>(splineTypeId);
      auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);

      spline->setSplineType(splineType);

      std::vector<sptr<EGEOM::Point>> points;

      auto pointsTbl = tbl["points"].as_array();

      pointsTbl->for_each([&points](auto &&el) {
        if constexpr (toml::is_table<decltype(el)>) {
          auto x = *el["x"].template value<float>();
          auto y = *el["y"].template value<float>();
          auto z = *el["z"].template value<float>();
          points.push_back(EGEOM::Point::create({x, y, z}));
        }
      });

      switch (splineType) {
      case EGEOM::Spline1::SplineType::LinearInterpolation: {
        auto linearBuilder =
            std::make_unique<EGEOM::LinearInterpolationBuilder>(
                points,
                EGEOM::LinearInterpolationBuilder::ParamMethod::Uniform);
        spline->setSplineBuilder(std::move(linearBuilder));
        spline->update();
      } break;
      case EGEOM::Spline1::SplineType::NURBS: {
        std::vector<float> knotVector;
        std::vector<float> weights;
        int power = *tbl["power"].value<int>();
        tbl["knotVector"].as_array()->for_each([&knotVector](auto &&el) {
          if constexpr (toml::is_floating_point<decltype(el)>) {
            knotVector.push_back(*el);
          }
        });
        tbl["weights"].as_array()->for_each([&weights](auto &&el) {
          if constexpr (toml::is_floating_point<decltype(el)>) {
            weights.push_back(*el);
          }
        });

        auto nurbsBuilder = std::make_unique<EGEOM::RationalBSplineBuilder>(
            points, power, knotVector, weights);
        spline->setSplineBuilder(std::move(nurbsBuilder));
        spline->update();
      } break;
      default: {
        spdlog::error("Error while loading spline from file");
        throw;
      }
      }

      auto newSketch = EGEOM::Sketch::create(
          "Sketch " + std::to_string(sketches.size()), spline);
      sketches.push_back(newSketch);
      currentSketchId = sketches.size() - 1;

      // action
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }

  std::vector<const char *> sketchesNameList;
  std::for_each(sketches.begin(), sketches.end(),
                [&sketchesNameList](auto sketch) {
                  sketchesNameList.push_back(sketch->name.c_str());
                });
  ImGui::ListBox("Sketches List", &currentSketchId, &sketchesNameList[0],
                 sketchesNameList.size(), 4);

  ImGui::End();
}

void MyApplication::handleObjectsGUI() {
  ImGui::Begin("Objects");
  std::vector<const char *> objectsNameList;
  std::for_each(viewportScene->getObjects().begin(),
                viewportScene->getObjects().end(),
                [&objectsNameList](auto object) {
                  objectsNameList.push_back(object->getName().c_str());
                });
  static int currentObject = 0;
  ImGui::ListBox("Objects List", &currentObject, &objectsNameList[0],
                 objectsNameList.size(), 4);
  ImGui::End();
}

void MyApplication::createPivotPlane() {
  auto pivotPlane = EGEOM::PivotPlane::create("PivotPlane 1");
  pivotPlane->isSelectable = true;

  pivotPlane->setPosition(viewportCamera->getPosition());

  viewportScene->addObject(pivotPlane);
}

void MyApplication::handlePropertiesGUI() {
  ImGui::Begin("Properties");
  if (activeWindow == Windows::Viewport) {
    if (selectedObjectViewport) {
      auto pivot =
          std::dynamic_pointer_cast<EGEOM::PivotPlane>(selectedObjectViewport);
      if (pivot)
        pivot->drawProperties(sketches);
      else
        selectedObjectViewport->drawProperties();
    } else
      ImGui::Text("Select object to edit properties...");
  }
  if (activeWindow == Windows::SketchEditor) {
    if (currentSketchId >= 0) {
      sketches[currentSketchId]->getSpline()->getPropertiesGUI(justSelected);
      justSelected = false;
    } else
      ImGui::Text("Select object to edit properties...");
  }
  ImGui::End();
}
