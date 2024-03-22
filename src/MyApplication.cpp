#include "MyApplication.hpp"
#include "BufferLayout.hpp"
#include "Renderer.hpp"
#include "VertexArray.hpp"
#include <Utilities.hpp>
#include <PivotPlane.hpp>
#include <memory>

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

    auto spline = EGEOM::Spline1::create(
            {}, EGEOM::Spline1::ParamMethod::Uniform, interpolationPointsCount);
    auto newSketch = EGEOM::Sketch::create(
            "Sketch " + std::to_string(sketches.size()), spline);
    sketches.push_back(newSketch);
    currentSketchId = sketches.size() - 1;

    auto grid = ENDER::Object::createGrid("Grid");

    viewportScene->addObject(grid);
    sketchScene->addObject(grid);
}

void MyApplication::handleViewportGUI() {
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
        for (auto object: viewportScene->getObjects()) {
            object->setSelected(object->getId() == pickedID);
            if (object->getId() == pickedID)
                selectedObjectViewport = object;
        }
    }

    ImGui::Image((ImTextureID) viewportFramebuffer->getTextureId(),
                 ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
//    if (ImGui::BeginPopupContextItem("popup")) // <-- use last item id as popup id
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
    if (ImGui::SliderFloat3("Rotation",
                            glm::value_ptr(rot), -4, 4)) {
        if (selectedObjectViewport)
            selectedObjectViewport->setRotation(rot);
    }


    for (auto obj: viewportScene->getObjects()) {
        if (obj->selected())
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("%s", obj->getName().c_str());
        if (obj->selected())
            ImGui::PopStyleColor();
    }
    for (auto light: viewportScene->getLights()) {
        if (light->type == ENDER::Light::LightType::PointLight)
            ImGui::Text("PointLight");
    }
    ImGui::End();
}

void MyApplication::handleSketchGUI() {
    ImGui::Begin("Sketch Editor");

    sketchCamera->setActive(ImGui::IsWindowFocused());

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
                              (ImVec4) ImColor::HSV(7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (ImVec4) ImColor::HSV(7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              (ImVec4) ImColor::HSV(7.0f, 0.8f, 0.8f));
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
                              (ImVec4) ImColor::HSV(7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (ImVec4) ImColor::HSV(7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              (ImVec4) ImColor::HSV(7.0f, 0.8f, 0.8f));
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

    endDockspace();
}

void MyApplication::update(float deltaTime) { viewportCamera->proccessInput(); }

void MyApplication::render() {
    ENDER::Renderer::renderScene(viewportScene, viewportFramebuffer);
    ENDER::Renderer::renderScene(sketchScene, sketchFramebuffer);

    if (currentSketchId == -1)
        return;

    if (renderDebugSplinePoints)
        for (auto p:
                sketches[currentSketchId]->getSpline()->getInterpolatedPoints()) {
            p->material.ambient = {0.0, 0.6, 0.6};
            p->material.diffuse = {0.0, 0.6, 0.6};

            ENDER::Renderer::renderObject(p, sketchScene, sketchFramebuffer);
        }

    for (auto p: sketches[currentSketchId]->getSpline()->getPoints()) {
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
            for (auto object: sketches[currentSketchId]->getSpline()->getPoints()) {
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
                    for (auto object:
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
        }
            break;
        case GLFW_KEY_L: {
            ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Lines);
        }
            break;
        case GLFW_KEY_N: {
            ENDER::Renderer::setRenderNormals(!ENDER::Renderer::isRenderingNormals());
        }
            break;
        case GLFW_KEY_K: {
            ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Triangles);
        }
            break;
        case GLFW_KEY_R: {
            currentOperation = ImGuizmo::OPERATION::ROTATE;
        }
            break;
        case GLFW_KEY_T: {
            currentOperation = ImGuizmo::OPERATION::TRANSLATE;
        }
            break;
        case GLFW_KEY_Y: {
            currentOperation = ImGuizmo::OPERATION::SCALE;
        }
            break;
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
        auto spline = EGEOM::Spline1::create(
                {}, EGEOM::Spline1::ParamMethod::Uniform, interpolationPointsCount);
        auto newSketch = EGEOM::Sketch::create(
                "Sketch " + std::to_string(sketches.size()), spline);
        sketches.push_back(newSketch);
        currentSketchId = sketches.size() - 1;
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
    if(selectedObjectViewport)
        selectedObjectViewport->drawProperties();
    else
        ImGui::Text("Select object to edit properties...");
    ImGui::End();
}
