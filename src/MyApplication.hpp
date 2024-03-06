#pragma once

#include "Line.hpp"
#include "Object.hpp"
#include "Point.hpp"
#include "imgui.h"
#include <Ender.hpp>
#include <ImGuizmo.h>
#include <memory>
#include <Spline1.hpp>
#include <Sketch.hpp>
#include <font/IconsFontAwesome5.h>
#include <PivotPlane.hpp>

class MyApplication : public ENDER::Application {

    enum class Tools{
        Cursor,
        Pencil
    };

    Tools currentTool = Tools::Cursor;
    bool mouseMove = false;

    bool renderDebugSplinePoints = false;

    sptr<ENDER::Framebuffer> viewportFramebuffer;
    sptr<ENDER::Framebuffer> sketchFramebuffer;

    sptr<ENDER::FirstPersonCamera> viewportCamera;
    sptr<ENDER::OrthographicCamera> sketchCamera;

    sptr<ENDER::Shader> lightCubeShader;

    sptr<ENDER::Scene> viewportScene;
    sptr<ENDER::Scene> sketchScene;

    sptr<EGEOM::PivotPlane> pivotPlane;

    std::vector<sptr<EGEOM::Sketch>> sketches;
    int currentSketchId = -1;

    sptr<EGEOM::Line> line;

    sptr<ENDER::Object> selectedObjectViewport;
    sptr<ENDER::Object> selectedObjectSketch;

    ImGuizmo::OPERATION currentOperation = ImGuizmo::OPERATION::TRANSLATE;

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
