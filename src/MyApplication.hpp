#pragma once

#include "Line.hpp"
#include "Object.hpp"
#include "Point.hpp"
#include "imgui.h"
#include <Ender.hpp>
#include <ImGuizmo.h>
#include <PivotPlane.hpp>
#include <Sketch.hpp>
#include <Spline1.hpp>
#include <font/IconsFontAwesome5.h>
#include <memory>

class MyApplication : public ENDER::Application {

  enum class Tools { Cursor, Pencil, Extrude, Rotate};

  enum class Windows { SketchEditor, Viewport };

  Tools currentTool = Tools::Cursor;
  Windows activeWindow = Windows::Viewport;
  
  bool mouseMove = false;
  bool justSelected = false;

  bool renderDebugSplinePoints = false;

  sptr<ENDER::Framebuffer> viewportFramebuffer;
  sptr<ENDER::Framebuffer> sketchFramebuffer;

  sptr<ENDER::FirstPersonCamera> viewportCamera;
  sptr<ENDER::OrthographicCamera> sketchCamera;

  sptr<ENDER::Shader> lightCubeShader;

  sptr<ENDER::Scene> viewportScene;
  sptr<ENDER::Scene> sketchScene;

  std::vector<sptr<EGEOM::Sketch>> sketches;
  int currentSketchId = -1;

  sptr<EGEOM::Line> line;

  sptr<ENDER::Object> selectedObjectViewport;
  sptr<ENDER::Object> selectedObjectSketch;

  ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;

  glm::vec3 extrudeDirection = {0,1,0};
  float extrudeHeight = 2;
  float rotateRadius = 1;
  float rotateAngle = glm::pi<float>()*2;

  uint _appWidth;
  uint _appHeight;

  int interpolationPointsCount = 30;

  ImVec2 sketchWindowPos;

  sptr<ENDER::Object> sphere;

  glm::vec3 directionalLightDirection;

  ENDER::DirectionalLight *directionalLight;

public:
  MyApplication(uint appWidth, uint appHeight);

  void onStart() override;

  static float degreeToRadians(float angle) {
    return angle * glm::pi<float>() / 180.0f;
  }

  static float radiansToDegree(float angle) {
    return angle / glm::pi<float>() * 180.0f;
  }

  void handleViewportGUI();
  void handleDebugGUI();
  void handleSketchGUI();
  void handleMenuBarGUI();
  void handleSketchSideGUI();
  void handleToolbarGUI();
  void handleObjectsGUI();
  void handlePropertiesGUI();
  void handleOperationPropertiesGUI();

  void createPivotPlane();

  void beginDockspace();

  void endDockspace();

  void onGUI() override;

  void update(float deltaTime) override;

  void render() override;

  void onKey(int key, ENDER::Window::EventStatus status) override;

  void onKeyRelease(int key) override;

  void onMouseClick(ENDER::Window::MouseButton button,
                    ENDER::Window::EventStatus status) override;

  void onClose();

  void onMouseMove(uint x, uint y);

  void onKeyPress(int key) override;
};
