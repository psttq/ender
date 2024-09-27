#include "MyApplication.hpp"
#include "ExtrudeSurface.hpp"
#include "IconsFontAwesome5.h"
#include "ImGuizmo.h"
#include "KinematicSurfaces.hpp"
#include "Object.hpp"
#include "Point.hpp"
#include "Renderer.hpp"
#include "RotationSurface.hpp"
#include "SectorialSurface.hpp"
#include "Sketch.hpp"
#include "Spline1.hpp"
#include "Topology/Edge.hpp"
#include "Topology/Face.hpp"
#include "Topology/Wire.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <PivotPlane.hpp>
#include <Utilities.hpp>
#include <fstream>
#include <glm/gtx/rotate_vector.hpp>
#include <optional>
#include <set>
#include <toml.hpp>

MyApplication::MyApplication(uint appWidth, uint appHeight)
    : ENDER::Application(appWidth, appHeight), _appWidth(appWidth),
      _appHeight(appHeight) {}

void MyApplication::onStart() {
  bool darkTheme = true;

  if (darkTheme) {
    ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});
    ENDER::Utils::applyImguiTheme();
  } else {
    ENDER::Renderer::setClearColor({1.f, 1.f, 1.f, 1.0f});
    ENDER::Utils::applyImguiWhiteTheme();
  }

  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("../resources/font.ttf", 14);

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
      ENDER::OrthographicCamera::create({2, 5, -1.4}, {_appWidth, _appHeight});

  viewportScene->setCamera(viewportCamera);
  sketchScene->setCamera(sketchCamera);

  directionalLightDirection = {-0.2f, -1.0f, -0.3f};

  directionalLight =
      new ENDER::DirectionalLight(directionalLightDirection, {1, 1, 1});
  viewportScene->addLight(directionalLight);
  sketchScene->addLight(directionalLight);

  //
  // sphere = ENDER::Utils::createParametricSurface(
  //     [](float u, float v) {
  //       return glm::vec3{glm::sin(u) * glm::cos(v), glm::sin(u) *
  //       glm::sin(v),
  //                        glm::cos(u)};
  //     },
  //     u_min, v_min, u_max, v_max, rows, cols);
  //
  // viewportScene->addObject(sphere);
  // sphere->isSelectable = true;
  //
  // auto s1 = ENDER::Utils::createParametricSurface(
  //     [](float u, float v) {
  //       return glm::vec3{glm::cos(u), glm::sin(u), v};
  //     },
  //     u_min, v_min, u_max, v_max, rows, cols);
  // s1->setPosition({0, -3, 3});
  // viewportScene->addObject(s1);
  //
  // auto cube = ENDER::Object::createCube("cube");
  // cube->setPosition({2, 0, 0});
  // cube->isSelectable = true;
  // viewportScene->addObject(cube);
  //
  auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);
  auto edge = EGEOM::Edge::create(spline);
  auto wire = EGEOM::Wire::create();
  wire->addEdge(edge);

  auto newSketch =
      EGEOM::Sketch::create("Sketch " + std::to_string(sketches.size()), wire);
  sketches.push_back(newSketch);
  currentSketchId = sketches.size() - 1;

  auto grid = ENDER::Object::createGrid("Grid");

  splineDim = EGEOM::Spline1::create({}, 100);
  splineDim->addPoint(EGEOM::Point::create({0, 0, 0}));
  splineDim->addPoint(EGEOM::Point::create({0, 2, 0}));
  splineDim->addPoint(EGEOM::Point::create({4, 3, 0}));
  splineDim->addPoint(EGEOM::Point::create({8, 4, 0}));
  splineDim->addPoint(EGEOM::Point::create({12, 5, 0}));
  splineDim->addPoint(EGEOM::Point::create({16, 4, 0}));
  splineDim->addPoint(EGEOM::Point::create({20, 3, 0}));
  splineDim->addPoint(EGEOM::Point::create({24, 2, 0}));

  dimSplines.push_back(splineDim);
  currentDimSpline = 0;
  // false
  //  std::vector<float> kv{};
  //  std::vector<float> wv{};
  //
  //  auto nurbsBuilder = std::make_unique<EGEOM::RationalBSplineBuilder>(
  //      splineDim->getPoints(), 4, kv, wv);
  //
  //  splineDim->setSplineType(EGEOM::Spline1::SplineType::NURBS);
  //  splineDim->setSplineBuilder(std::move(nurbsBuilder));
  //
  viewportScene->addObject(splineDim);

  viewportScene->addObject(grid);
  sketchScene->addObject(grid);
}

void MyApplication::handleOperationPropertiesGUI() {
  if (currentTool == Tools::Extrude) {
    ImGui::Begin("Extrude");

    ImGui::DragFloat3("Extrude Direction", glm::value_ptr(extrudeDirection),
                      0.1f, -1, 1);
    ImGui::DragFloat("Extrude Height", &extrudeHeight, 0.1);
    if (ImGui::Button("Create")) {

      extrudeDirection = glm::normalize(extrudeDirection);

      auto pivot =
          std::dynamic_pointer_cast<EGEOM::PivotPlane>(selectedObjectViewport);
      if (pivot && pivot->getSketch()) {
        auto wire = pivot->getSketch()->getWire();
        auto edges = wire->getEdges();

        auto bottom = EGEOM::SectorialSurface::create(
            edges[0]->getSpline()->getPoints()[0], wire);

        bottom->isSelectable = true;
        bottom->update();

        auto bottom_face = EGEOM::Face::create(bottom, wire);
        bottom_face->isSelectable = true;
        bottom_face->setPosition(pivot->getPosition());
        bottom_face->setRotation(pivot->getRotation());

        viewportScene->addObject(bottom_face);

        auto upperWire = EGEOM::Wire::create();

        sptr<EGEOM::Face> prevFace;
        sptr<EGEOM::Edge> firstSideEdge;

        for (auto edge : edges) {
          auto obj =
              EGEOM::ExtrudeSurface::create("ExtrudeSurface", edge->getSpline(),
                                            extrudeDirection, extrudeHeight);

          obj->isSelectable = true;

          auto beginPoint = *edge->getSpline()->getPoints().begin();

          auto wire = EGEOM::Wire::create();
          auto bottomEdge = edge->copy();
          bottomEdge->isInvertedDirection = true;
          wire->addEdge(bottomEdge);

          auto face = EGEOM::Face::create(obj, wire);
          face->setPosition(selectedObjectViewport->getPosition());
          face->setRotation(selectedObjectViewport->getRotation());
          face->isSelectable = true;

          edge->isSelectable = true;

          auto upperEdge = edge->clone();

          for (auto point : upperEdge->getSpline()->getPoints()) {
            point->setPosition(point->getPosition() +
                               extrudeHeight * extrudeDirection);
          }
          upperEdge->setName("upperEdge");
          upperEdge->update();

          upperWire->addEdge(upperEdge);

          auto endPoint = *upperEdge->getSpline()->getPoints().begin();
          auto sideEdge = EGEOM::Edge::create(
              EGEOM::Spline1::create({beginPoint, endPoint}, 10));
          face->addEdge(sideEdge);

          if (!firstSideEdge)
            firstSideEdge = sideEdge;

          if (prevFace) {
            auto sideEdgeCopy = sideEdge->copy();
            sideEdgeCopy->isInvertedDirection = true;
            prevFace->addEdge(sideEdgeCopy);
          }
          prevFace = face;

          face->addEdge(upperEdge);

          viewportScene->addObject(face);
        }
        auto edge = firstSideEdge->copy();
        edge->isInvertedDirection = true;
        prevFace->addEdge(edge);

        auto edgesUpper = upperWire->getEdges();
        auto upper = EGEOM::SectorialSurface::create(
            edgesUpper[0]->getSpline()->getPoints()[0], upperWire);

        upper->isSelectable = true;
        upper->update();

        auto upper_face = EGEOM::Face::create(upper, upperWire);
        upper_face->isSelectable = true;
        upper_face->setPosition(pivot->getPosition());
        upper_face->setRotation(pivot->getRotation());

        viewportScene->addObject(upper_face);
      }
    }
    ImGui::End();
  } else if (currentTool == Tools::Rotate) {
    ImGui::Begin("Rotate");
    ImGui::DragFloat("Rotate radius", &rotateRadius, 0.1);
    ImGui::DragFloat("Rotate angle", &rotateAngle, 0.1, 0,
                     glm::pi<float>() * 2);
    if (ImGui::Button("Create")) {
      auto pivot =
          std::dynamic_pointer_cast<EGEOM::PivotPlane>(selectedObjectViewport);
      if (pivot && pivot->getSketch()) {
        auto wire = pivot->getSketch()->getWire();
        auto edges = wire->getEdges();

        std::set<sptr<EGEOM::Point>> edgesPoints;

        for (auto edge : edges) {
          auto obj = EGEOM::RotationSurface::create(edge->getName() + "_RS",
                                                    edge->getSpline(),
                                                    rotateAngle, rotateRadius);
          obj->setPosition(selectedObjectViewport->getPosition());
          obj->setRotation(selectedObjectViewport->getRotation());
          obj->isSelectable = true;

          edgesPoints.insert(*edge->getSpline()->getPoints().begin());
          edgesPoints.insert(*(edge->getSpline()->getPoints().end() - 1));

          viewportScene->addObject(obj);
        }
        for (auto edgePoint : edgesPoints) {
          auto point = edgePoint->getPosition();

          auto newEdge = EGEOM::Spline1::create({}, 200);

          newEdge->setName("EDGE");

          newEdge->setSplineType(EGEOM::Spline1::SplineType::Parametric);
          auto parametricBuilder = uptr<EGEOM::ParametricBuilder>(
              new EGEOM::ParametricBuilder([&](float v) {
                auto splinePoint = point + glm::vec3{-rotateRadius, 0, 0};
                return glm::vec3{rotateRadius, 0, 0} +
                       glm::vec3{splinePoint.x * glm::cos(v),
                                 splinePoint.x * glm::sin(v), splinePoint.z};
              }));
          newEdge->u_max = rotateAngle;
          newEdge->isSelectable = true;
          newEdge->setSplineBuilder(std::move(parametricBuilder));
          newEdge->setPosition(selectedObjectViewport->getPosition());
          newEdge->setRotation(selectedObjectViewport->getRotation());
          newEdge->update();
          viewportScene->addObject(newEdge);
        }
      }
    }
    ImGui::End();
  } else if (currentTool == Tools::Kinematic) {
    ImGui::Begin("KinematicSurface");
    std::vector<const char *> items = {"Sweep", "Shift"};
    int currentKinematicSurfaceType = 0;
    ImGui::Combo("Kinematic Surface Type", &currentKinematicSurfaceType,
                 &items[0], items.size());
    if (ImGui::Button("Create"))
      if (currentSketchId >= 0 && currentDimSpline >= 0) {
        auto formingSpline =
            sketches[currentSketchId]->getWire()->getEdges()[0]->getSpline();
        auto guideSpline = dimSplines[currentDimSpline];
        if (formingSpline->getSplineType() ==
                EGEOM::Spline1::SplineType::NURBS &&
            guideSpline->getSplineType() == EGEOM::Spline1::SplineType::NURBS) {
          auto surfType =
              static_cast<EGEOM::KinematicSurface::KinematicSurfaceType>(
                  currentKinematicSurfaceType);
          auto obj = EGEOM::KinematicSurface::create(
              "Kinematic surface", formingSpline, guideSpline, surfType);
          // obj->setPosition(guideSpline->getPoints()[0]->getPosition());
          viewportScene->addObject(obj);
        }
      }
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
  ImVec2 screen_pos = ImGui::GetCursorScreenPos();
  auto mousePosition = ENDER::Window::getMousePosition();
  auto objInfo = viewportFramebuffer->pickObjAt(
      mousePosition.x - screen_pos.x, (mousePosition.y - screen_pos.y));

  bool hoverWithParent = false;
  auto pickedID = objInfo.parentId != 0 ? objInfo.parentId : objInfo.objectId;
  for (auto object : viewportScene->getObjects()) {
    // spdlog::error("pickedID {}{} {} {} {}", object->getName(),
    // object->getId(), objInfo.objectId,
    // object->hasChildById(objInfo.objectId), object->getId() ==
    // objInfo.objectId);
    if (hoverWithParent) {
      object->setHovered(object->getId() == pickedID);
    } else {
      object->setHovered(object->getId() == objInfo.objectId);
      object->getChildByID(objInfo.objectId).and_then([](auto obj) {
        obj->setHovered(true);
        return std::optional(obj);
      });
    }
  }
  if (hoverWithParent)
    viewportScene->getObjectById(pickedID).and_then(
        [](sptr<ENDER::Object> obj) {
          obj->setHovered(true);
          return std::optional(obj);
        });

  if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left) &&
      ImGui::IsWindowFocused() && !ImGuizmo::IsUsing()) {
    auto pickedID = objInfo.parentId != 0 ? objInfo.parentId : objInfo.objectId;
    // auto pickedID = objInfo.objectId;
    spdlog::error("PICK {}", pickedID);
    for (auto object : viewportScene->getObjects()) {
      object->setSelected(object->getId() == pickedID);
      if (object->getId() == pickedID)
        selectedObjectViewport = object;
    }
  }

  ImGui::Image(
      reinterpret_cast<ImTextureID>(viewportFramebuffer->getTextureId()),
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

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                         currentOperation, ImGuizmo::LOCAL,
                         glm::value_ptr(model));

    float h = 1.0f / numCount;

    auto hs = [](float v) { return glm::vec3{v + 1000, v + 1000, v + 1000}; };

    if (splineDim->getSplineType() == EGEOM::Spline1::SplineType::NURBS)
      for (auto i = 0; i <= numCount; ++i) {
        float u = h * i;

        auto gs = splineDim->getSplineDirs(u, 3);

        if (gs.size() < 3)
          break;

        auto g2 = gs[2]->getPosition();
        // glm::vec3 d = glm::normalize(
        //     glm::cross(gs[1]->getPosition(), gs[0]->getPosition()));
        //
        glm::vec3 d = gs[1]->getPosition() - hs(u);
        auto i1 = glm::normalize(gs[1]->getPosition());
        auto d2 = d - (glm::dot(i1, d)) * i1;
        auto i2 = glm::normalize(d2);
        auto i3 = glm::cross(i1, i2);

        auto dr = glm::normalize(d);

        auto p1 = gs[0]->getPosition();
        auto p2 = p1 + i1;
        ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                            0xFFaaaa55);
        ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0},
                            {p1.x + dr.x, p1.y + dr.y, p1.z + dr.z, 0},
                            0xFFFFFFFF);
        auto p3 = p1 + i2;
        ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p3.x, p3.y, p3.z, 0},
                            0xAAFFaa55);
        auto p4 = p1 + i3;
        ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p4.x, p4.y, p4.z, 0},
                            0xAAAAFF55);
      }

    if (currentTool == Tools::Extrude &&
        selectedObjectViewport->label == "PivotPlane") {
      glm::vec3 p1 = selectedObjectViewport->getPosition();
      auto dir = extrudeDirection / glm::length(extrudeDirection);
      auto p2 = p1 + dir * extrudeHeight;

      auto diff = p2 - p1;
      glm::vec3 xNorm(1.0, 0.0f, 0.0);
      glm::vec3 yNorm(0.0, 1.0f, 0.0);
      glm::vec3 zNorm(0.0, 0.0f, 1.0);

      auto _rotation = selectedObjectViewport->getRotation();

      diff = glm::rotate(diff, _rotation.x, xNorm); // Rotate on X axis
      diff = glm::rotate(diff, _rotation.y, yNorm); // Rotate on Y axis
      diff = glm::rotate(diff, _rotation.z, zNorm); // Rotate

      p2 = p1 + diff;

      ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                          0xFF110055);
    } else if (currentTool == Tools::Rotate &&
               selectedObjectViewport->label == "PivotPlane") {
      glm::vec3 p1 = selectedObjectViewport->getPosition();
      p1 += glm::vec3{1, 0, 0} * rotateRadius;
      auto p2 = p1 + glm::vec3{0, 0, 1};

      auto diff = p2 - p1;
      glm::vec3 xNorm(1.0, 0.0f, 0.0);
      glm::vec3 yNorm(0.0, 1.0f, 0.0);
      glm::vec3 zNorm(0.0, 0.0f, 1.0);

      auto _rotation = selectedObjectViewport->getRotation();

      diff = glm::rotate(diff, _rotation.x, xNorm); // Rotate on X axis
      diff = glm::rotate(diff, _rotation.y, yNorm); // Rotate on Y axis
      diff = glm::rotate(diff, _rotation.z, zNorm); // Rotate

      p2 = p1 + diff;

      ImGuizmo::DrawArrow({p1.x, p1.y, p1.z, 0}, {p2.x, p2.y, p2.z, 0},
                          0xFF110055);
    }

    // for (auto &obj : viewportScene->getObjects()) {
    //   obj->drawGizmo();
    // }

    selectedObjectViewport->drawGizmo();

    if (ImGuizmo::IsUsing()) {

      glm::vec3 newPosition;
      glm::vec3 newRotation;
      glm::vec3 newScale;

      ImGuizmo::DecomposeMatrixToComponents(
          glm::value_ptr(model), glm::value_ptr(newPosition),
          glm::value_ptr(newRotation), glm::value_ptr(newScale));

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
    sketches[currentSketchId]
        ->getWire()
        ->getCurrentEdge()
        ->getSpline()
        ->setInterpolationPointsCount(interpolationPointsCount);
    sketches[currentSketchId]->getWire()->getCurrentEdge()->update();
  }

  if (ImGui::SliderFloat3("DirectionalLight",
                          glm::value_ptr(directionalLightDirection), -1, 1)) {
    directionalLight->setDirection(directionalLightDirection);
  }

  ImGui::SliderInt("Vecs", &numCount, 0, 400);

  ImGui::End();
}

void MyApplication::handleSketchGUI() {
  ImGui::Begin("Sketch Editor");

  sketchCamera->setActive(ImGui::IsWindowFocused());

  if (ImGui::IsWindowFocused()) {
    activeWindow = Windows::SketchEditor;
  }
  float window_width = ImGui::GetContentRegionAvail().x;
  float window_height = ImGui::GetContentRegionAvail().y;

  sketchCamera->setFramebufferSize({window_width, window_height});

  sketchFramebuffer->rescale(window_width, window_height);

  sketchWindowPos = ImGui::GetCursorScreenPos();

  ImGui::Image(reinterpret_cast<ImTextureID>(sketchFramebuffer->getTextureId()),
               ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
  if (currentSketchId >= 0) {
    auto obj = sketches[currentSketchId]->getWire()->getCurrentEdge();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                      window_width, window_height);

    auto objRotation = obj->getRotation();

    glm::mat4 model;
    auto rotation = obj->getRotation();
    rotation = {radiansToDegree(rotation.x), radiansToDegree(rotation.y),
                radiansToDegree(rotation.z)};
    ImGuizmo::RecomposeMatrixFromComponents(
        glm::value_ptr(obj->getPosition()), glm::value_ptr(rotation),
        glm::value_ptr(obj->getScale()), glm::value_ptr(model));

    auto cameraView = sketchCamera->getView();
    auto cameraProj = sketchCamera->getProjection();

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProj),
                         currentOperation, ImGuizmo::LOCAL,
                         glm::value_ptr(model));

    obj->drawGizmo();
  }

  ImGui::End();
}

void MyApplication::handleMenuBarGUI() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      if (ImGui::MenuItem("Close", NULL, false, true))
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

  ImGui::SameLine();
  if (currentTool == Tools::Spliner) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
    setStyle = true;
  }
  if (ImGui::Button(ICON_FA_BEZIER_CURVE)) {
    currentTool = Tools::Spliner;
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
  ImGui::SameLine();
  if (currentTool == Tools::Kinematic) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
    setStyle = true;
  }
  if (ImGui::Button(ICON_FA_LAYER_GROUP)) {
    currentTool = Tools::Kinematic;
  }
  ImGui::SetItemTooltip("Kinematic Surface Tool");

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
  handleDimensionalSplinesGUI();

  endDockspace();
}

void MyApplication::update(float deltaTime) { viewportCamera->proccessInput(); }

void MyApplication::render() {
  ENDER::Renderer::renderScene(viewportScene, viewportFramebuffer);
  ENDER::Renderer::renderScene(sketchScene, sketchFramebuffer);

  if (currentSketchId == -1)
    return;

  if (renderDebugSplinePoints)
    for (auto p : sketches[currentSketchId]
                      ->getWire()
                      ->getCurrentEdge()
                      ->getSpline()
                      ->getInterpolatedPoints()) {
      p->material.ambient = {0.0, 0.6, 0.6};
      p->material.diffuse = {0.0, 0.6, 0.6};

      ENDER::Renderer::renderObject(p, sketchScene, sketchFramebuffer);
    }

  for (auto edge : sketches[currentSketchId]->getWire()->getEdges()) {
    for (auto p : edge->getSpline()->getPoints()) {
      if (sketches[currentSketchId]->getWire()->isCurrentEdge(edge)) {
        p->material.ambient = {0.5, 0.3, 0.7};
        p->material.diffuse = {0.5, 0.3, 0.7};
        p->material.specular = {0.5, 0.3, 0.7};
      } else {
        p->material.ambient = {1.0f, 0.5f, 0.31f};
        p->material.diffuse = {1.0f, 0.5f, 0.31f};
        p->material.specular = {0.5f, 0.5f, 0.5f};
      }
      ENDER::Renderer::renderObject(p, sketchScene, sketchFramebuffer);
    }
    if (sketches[currentSketchId]->getWire()->isCurrentEdge(edge)) {
      edge->material.ambient = {0.3, 0.3, 0.6};
      edge->material.diffuse = {0.3, 0.3, 0.6};
      edge->material.specular = {0.3, 0.3, 0.6};
    } else {
      edge->material.ambient = {1.0f, 0.5f, 0.31f};
      edge->material.diffuse = {1.0f, 0.5f, 0.31f};
      edge->material.specular = {0.5f, 0.5f, 0.5f};
    }
    edge->isSelectable = true;
    ENDER::Renderer::renderObject(edge, sketchScene, sketchFramebuffer);
  }
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
  if (button == ENDER::Window::MouseButton::Right &&
      status == ENDER::Window::EventStatus::Press && sketchCamera->isActive()) {
    if (currentSketchId != -1) {
      auto mousePosition = ENDER::Window::getMousePosition();

      auto mouseScreenPosX = mousePosition.x - sketchWindowPos.x;
      auto mouseScreenPosY = mousePosition.y - sketchWindowPos.y;

      if (mouseScreenPosX < 0 || mouseScreenPosY < 0)
        return;

      sptr<ENDER::Object> currentSelected;
      auto pickedID =
          sketchFramebuffer->pickObjAt(mouseScreenPosX, mouseScreenPosY)
              .objectId;
      for (auto object : sketches[currentSketchId]
                             ->getWire()
                             ->getCurrentEdge()
                             ->getSpline()
                             ->getPoints()) {

        object->setSelected(object->getId() == pickedID);
        if (object->getId() == pickedID) {
          currentSelected = object;
        }
      }
      if (currentTool == Tools::Pencil) {
        sketches[currentSketchId]
            ->getWire()
            ->getCurrentEdge()
            ->getSpline()
            ->removePoint(
                std::static_pointer_cast<EGEOM::Point>(currentSelected));
      }
      sketches[currentSketchId]->getWire()->getCurrentEdge()->update();
    }
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
          sketchFramebuffer->pickObjAt(mouseScreenPosX, mouseScreenPosY)
              .objectId;
      spdlog::error("ppp {}", pickedID);

      for (auto edge : sketches[currentSketchId]->getWire()->getEdges()) {
        for (auto object : edge->getSpline()->getPoints()) {
          object->setSelected(object->getId() == pickedID);
          if (object->getId() == pickedID) {
            currentSelected = object;
            if (currentTool == Tools::Cursor) {
              sketches[currentSketchId]->getWire()->setCurrentEdge(edge);
            }
          }
        }
        if (edge->getId() == pickedID) {
          if (currentTool == Tools::Cursor)
            sketches[currentSketchId]->getWire()->setCurrentEdge(edge);
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
        sketches[currentSketchId]
            ->getWire()
            ->getCurrentEdge()
            ->getSpline()
            ->addPoint(point);
      } else if (currentTool == Tools::Cursor) {
        if (currentSelected) {
          selectedObjectSketch = currentSelected;
          justSelected = true;
          for (auto object : sketches[currentSketchId]
                                 ->getWire()
                                 ->getCurrentEdge()
                                 ->getSpline()
                                 ->getPoints())
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

    // auto lightCube = ENDER::Object::createCube("Light Debug Cube");
    // lightCube->setPosition(pos);
    // lightCube->setScale(glm::vec3(0.2f));
    // lightCube->setShader(lightCubeShader);
    // viewportScene->addObject(lightCube);
    //
    // auto pointLight = new ENDER::PointLight(pos, glm::vec3(1));
    // viewportScene->addLight(pointLight);
    if (currentDimSpline >= 0 && currentTool == Tools::Spliner) {
      auto point = EGEOM::Point::create(pos);
      point->setScale({0.4, 0.4, 0.4});
      point->isSelectable = true;
      viewportScene->addObject(point);
      dimSplines[currentDimSpline]->addPoint(point);
    }
  } break;
  case GLFW_KEY_P: {
    auto pos = viewportScene->getCamera()->getPosition();
    viewportScene->addLight(new ENDER::PointLight(pos, glm::vec3{1}));
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
    auto currSpline = sketches[currentSketchId]->getWire()->getEdges()[0];
    auto gs0 = splineDim->getSplineDirs(0, 3);

    auto hs = [](float v) {
      return glm::vec3{v + 1000 * v * v, v - 1000 * v, v * v + 1000};
    };
    auto i10 = glm::normalize(gs0[1]->getPosition());
    auto d20 = hs(0) - (glm::dot(i10, hs(0))) * i10;
    auto i20 = glm::normalize(d20);
    auto i30 = glm::cross(i10, i20);

    glm::mat3 Am{i10, i20, i30};
    Am = glm::inverse(Am);

    auto obj = ENDER::Utils::createParametricSurface(
        [&](float u, float v) {
          auto g = splineDim->getSplineDirs(v, 1)[0]->getPosition();
          auto g0 = splineDim->getSplinePoint(0);
          auto c = currSpline->getSpline()->getSplinePoint(u);
          glm::vec3 h = {0, 0, 0};
          auto p = g + (c - g0 - h);
          return p;

          // auto gs = splineDim->getSplineDirs(v, 3);
          //
          // glm::vec3 d = gs[0]->getPosition() - hs(v);
          // //
          // // glm::vec3 d = glm::normalize(
          // //     glm::cross(gs[1]->getPosition(), gs[2]->getPosition()));
          //
          // auto i1 = glm::normalize(gs[1]->getPosition());
          // auto d2 = d - (glm::dot(i1, d)) * i1;
          // auto i2 = glm::normalize(d2);
          // auto i3 = glm::cross(i1, i2);
          //
          // glm::mat3 A{i1, i2, i3};
          //
          // // A = glm::transpose(A);
          //
          // auto M = A * Am;
          //
          // // spdlog::info("M[][0] = {} {} {}", M[0][0], M[1][0], M[2][0]);
          // // spdlog::info("M[][1] = {} {} {}", M[0][1], M[1][1], M[2][1]);
          // // spdlog::info("M[][2] = {} {} {}", M[0][2], M[1][2], M[2][2]);
          // //
          // auto g = gs[0]->getPosition();
          // auto g0 = gs0[0]->getPosition();
          // auto c = currSpline->getSplinePoint(u);
          // glm::vec3 h = {0, 0, 0};
          // auto p = g + M * (c - g0 - h);
          // return p;
        },
        0, 0, 1, 1, 600, 600);
    obj->isSelectable = true;
    viewportScene->addObject(obj);
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
      sketches[currentSketchId]->getWire()->update();
    }
  }
}

void MyApplication::handleDimensionalSplinesGUI() {
  ImGui::Begin("DimSplines");
  std::vector<const char *> objectsNameList;

  for (auto &spline : dimSplines) {
    objectsNameList.push_back(spline->getName().c_str());
  }

  if (ImGui::ListBox("Spline List", &currentDimSpline, &objectsNameList[0],
                     objectsNameList.size(), 4)) {
    auto selectedObj = dimSplines[currentDimSpline];
    selectedObjectViewport = selectedObj;
    for (auto &obj : viewportScene->getObjects()) {
      obj->setSelected(false);
    }
    selectedObj->setSelected(true);
  }
  if (ImGui::Button("Create")) {
    auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);
    spline->addPoint(EGEOM::Point::create({0, 0, 0}));
    viewportScene->addObject(spline);
    dimSplines.push_back(spline);
  }
  ImGui::End();
}

void MyApplication::handleSketchSideGUI() {
  ImGui::Begin("Sketches");
  if (ImGui::Button("Add sketch")) {
    auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);
    auto edge = EGEOM::Edge::create(spline);
    auto wire = EGEOM::Wire::create();
    wire->addEdge(edge);

    auto newSketch = EGEOM::Sketch::create(
        "Sketch " + std::to_string(sketches.size()), wire);
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

      toml::array wires;
      toml::table result_table;

      auto currentSketch = sketches[currentSketchId];

      std::set<sptr<EGEOM::Point>> points_set;
      for (auto edge : currentSketch->getWire()->getEdges()) {
        for (auto point : edge->getSpline()->getPoints()) {
          points_set.insert(point);
        }
      }

      toml::array points;
      for (auto &p : points_set) {
        auto position = p->getPosition();
        auto pointTable = toml::table{
            {"x", position.x}, {"y", position.y}, {"z", position.z}};

        points.push_back(pointTable);
      }

      for (auto edge : currentSketch->getWire()->getEdges()) {
        auto spline = edge->getSpline();
        toml::table tbl;

        toml::array spline_points;

        for (auto point : spline->getPoints()) {
          int i = 0;
          for (auto ps : points_set) {
            if (ps == point)
              break;
            i++;
          }
          spline_points.push_back(i);
        }

        switch (spline->getSplineType()) {
        case EGEOM::Spline1::SplineType::LinearInterpolation: {

          tbl = toml::table{{"type", static_cast<int>(spline->getSplineType())},
                            {"points", spline_points}};
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
                            {"points", spline_points}};
        } break;
        default: {
          spdlog::error("Can't write sketch of this type!");
        }
        }
        wires.push_back(tbl);
      }
      result_table = toml::table{{"points_set", points}, {"wires", wires}};
      if (!result_table.empty()) {
        std::ofstream fs(filePathName);
        fs << result_table;
        spdlog::info("Sketch successefully written to {}", filePathName);
      }
      // action
    }
    ImGuiFileDialog::Instance()->Close();
  }
  // close

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

      std::vector<sptr<EGEOM::Point>> points;

      auto pointsTbl = tbl["points_set"].as_array();

      pointsTbl->for_each([&points](auto &&el) {
        if constexpr (toml::is_table<decltype(el)>) {
          auto x = *el["x"].template value<float>();
          auto y = *el["y"].template value<float>();
          auto z = *el["z"].template value<float>();
          auto point = EGEOM::Point::create({x, y, z});
          point->isSelectable = true;
          points.push_back(point);
        }
      });

      spdlog::info("Points loaded");

      auto wiresTbl = tbl["wires"].as_array();
      auto wire = EGEOM::Wire::create();

      wiresTbl->for_each([&](auto &&el) {
        if constexpr (toml::is_table<decltype(el)>) {

          auto splineTypeId = *el["type"].template value<int>();
          spdlog::info("Spline type: {}", splineTypeId);

          auto splineType =
              static_cast<EGEOM::Spline1::SplineType>(splineTypeId);
          auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);

          spline->setSplineType(splineType);

          std::vector<sptr<EGEOM::Point>> spline_points;
          el["points"].as_array()->for_each(
              [&spline_points, &points](auto &&el) {
                if constexpr (toml::is_integer<decltype(el)>) {
                  spdlog::info("Knot vector: {}", *el);
                  spline_points.push_back(points[*el]);
                }
              });

          switch (splineType) {
          case EGEOM::Spline1::SplineType::LinearInterpolation: {
            auto linearBuilder =
                std::make_unique<EGEOM::LinearInterpolationBuilder>(
                    spline_points,
                    EGEOM::LinearInterpolationBuilder::ParamMethod::Uniform);
            spline->setSplineBuilder(std::move(linearBuilder));
            spline->update();
          } break;
          case EGEOM::Spline1::SplineType::NURBS: {
            std::vector<float> knotVector;
            std::vector<float> weights;

            int power = *el["power"].template value<int>();
            el["knotVector"].as_array()->for_each([&knotVector](auto &&el) {
              if constexpr (toml::is_floating_point<decltype(el)>) {
                spdlog::info("Knot vector: {}", *el);
                knotVector.push_back(*el);
              }
            });
            el["weights"].as_array()->for_each([&weights](auto &&el) {
              if constexpr (toml::is_floating_point<decltype(el)>) {
                weights.push_back(*el);
              }
            });

            auto nurbsBuilder = std::make_unique<EGEOM::RationalBSplineBuilder>(
                spline_points, power, knotVector, weights);
            spline->setSplineBuilder(std::move(nurbsBuilder));
            spline->update();
          } break;
          default: {
            spdlog::error("Error while loading spline from file");
            throw;
          }
          }
          wire->addEdge(EGEOM::Edge::create(spline));
        }
      });

      auto newSketch = EGEOM::Sketch::create(
          "Sketch " + std::to_string(sketches.size()), wire);
      sketches.push_back(newSketch);
      currentSketchId = sketches.size() - 1;

      // action
    }

    // close
    ImGuiFileDialog::Instance()->Close();
  }

  // std::vector<const char *> sketchesNameList;
  // std::for_each(sketches.begin(), sketches.end(),
  //               [&sketchesNameList](auto sketch) {
  //                 sketchesNameList.push_back(sketch->name.c_str());
  //               });
  // ImGui::ListBox("Sketches List", &currentSketchId, &sketchesNameList[0],
  //                sketchesNameList.size(), 4);

  if (ImGui::Button("Add Spline")) {
    auto currentSketch = sketches[currentSketchId];
    auto wire = currentSketch->getWire();
    auto spline = EGEOM::Spline1::create({}, interpolationPointsCount);
    auto edge = EGEOM::Edge::create(spline);
    wire->addEdge(edge);
    wire->currentEdge = wire->getEdges().size() - 1;
  }

  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;

  if (ImGui::TreeNode("Sketches")) {
    auto i = 0;
    for (auto &sketch : sketches) {
      ImGuiTreeNodeFlags node_flags = base_flags;
      if (i == currentSketchId)
        node_flags |= ImGuiTreeNodeFlags_Selected;
      bool node_open =
          ImGui::TreeNodeEx((void *)(intptr_t)sketch->getId(), node_flags, "%s",
                            sketch->getName().c_str());
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        currentSketchId = i;
      }
      if (node_open) {
        for (auto child : sketch->getWire()->getChildren()) {
          if (child == sketch->getWire()->getCurrentEdge()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("%s %d", child->getName().c_str(), child->getId());
            ImGui::PopStyleColor();
          } else {
            ImGui::Text("%s %d", child->getName().c_str(), child->getId());
          }
        }
        ImGui::TreePop();
      }
      i++;
    }
    ImGui::TreePop();
  }

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
  if (ImGui::ListBox("Objects List", &currentObject, &objectsNameList[0],
                     objectsNameList.size(), 4)) {
    auto selectedObj = viewportScene->getObjects()[currentObject];
    selectedObjectViewport = selectedObj;
    for (auto &obj : viewportScene->getObjects()) {
      obj->setSelected(false);
    }
    selectedObj->setSelected(true);
  }
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;

  ImGuiTreeNodeFlags node_flags = base_flags;

  if (ImGui::TreeNode("Objects")) {
    for (auto &obj : viewportScene->getObjects()) {
      if (obj->selected())
        node_flags |= ImGuiTreeNodeFlags_Selected;
      bool node_open =
          ImGui::TreeNodeEx((void *)(intptr_t)obj->getId(), node_flags, "%s",
                            obj->getName().c_str());
      if (node_open) {
        for (auto child : obj->getChildren()) {
          bool child_open =
              ImGui::TreeNodeEx((void *)(intptr_t)child->getId(), node_flags,
                                "%s", child->getName().c_str());
          if (child_open) {
            ImGui::TreePop();
          }
        }
        ImGui::TreePop();
      }
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selectedObjectViewport = obj;
        for (auto &obj : viewportScene->getObjects()) {
          obj->setSelected(false);
        }
        obj->setSelected(true);
      }
    }
    ImGui::TreePop();
  }
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
      auto spline =
          std::dynamic_pointer_cast<EGEOM::Spline1>(selectedObjectViewport);
      if (pivot)
        pivot->drawProperties(sketches);
      else if (spline) {
        spline->getPropertiesGUI(justSelected);
        justSelected = false;
      } else
        selectedObjectViewport->drawProperties();
    } else
      ImGui::Text("Select object to edit properties...");
  }
  if (activeWindow == Windows::SketchEditor) {
    if (currentSketchId >= 0) {
      sketches[currentSketchId]
          ->getWire()
          ->getCurrentEdge()
          ->getSpline()
          ->getPropertiesGUI(justSelected);
      justSelected = false;
    } else
      ImGui::Text("Select object to edit properties...");
  }
  ImGui::End();
}
