#pragma once

#include "Line.hpp"
#include "Object.hpp"
#include "Point.hpp"
#include "imgui.h"
#include <Ender.hpp>
#include <ImGuizmo.h>
#include <memory>
#include <Spline1.hpp>
#include <font/IconsFontAwesome5.h>

class MyApplication : public ENDER::Application {

    enum class Tools{
        Cursor,
        Pencil
    };

    Tools currentTool = Tools::Cursor;

    sptr<ENDER::Framebuffer> viewportFramebuffer;
    sptr<ENDER::Framebuffer> sketchFramebuffer;

    sptr<ENDER::FirstPersonCamera> viewportCamera;
    sptr<ENDER::OrthographicCamera> sketchCamera;

    sptr<ENDER::Shader> lightCubeShader;

    sptr<ENDER::Scene> viewportScene;
    sptr<ENDER::Scene> sketchScene;

    std::vector<sptr<EGEOM::Point>> points;

    sptr<EGEOM::Line> line;
    sptr<EGEOM::Spline1> spline1;

    sptr<ENDER::Object> selectedObjectViewport;

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

    void beginDockspace();

    void endDockspace();

    void handleToolbarGUI();

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
