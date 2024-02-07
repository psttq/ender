#include <Ender.hpp>
#include <Utilities.hpp>
#include <memory>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

int main() {
    spdlog::set_level(spdlog::level::debug);

    float u_max = 2 * glm::pi<float>();
    float v_max = 2 * glm::pi<float>();

    int rows = 20;
    int cols = 20;

    ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

    ENDER::Renderer::init();

    auto framebuffer = ENDER::Framebuffer::create(SCR_WIDTH, SCR_HEIGHT);

    auto lightCubeShader = ENDER::Shader::create("../resources/shaders/lightShader.vs",
                                                 "../resources/shaders/lightShader.fs");

    auto scene = ENDER::Scene::create();

    auto camera = ENDER::FirstPersonCamera::create({});

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
            return glm::vec3{
                glm::sin(u) * glm::cos(v), glm::sin(u) * glm::sin(v),
                glm::cos(u)
            };
        },
        u_max, v_max, rows, cols);
    scene->addObject(sphere);

    // auto sphere = ENDER::Utils::createParametricSurface(
    //   [](float u, float v) {
    //     return glm::vec3{glm::sin(u), glm::cos(v),
    //                      u};
    //   },
    //   u_max, v_max, rows, cols);
    // scene->addObject(sphere);

    auto cube = ENDER::Object::createCube("cube");
    cube->setPosition({2, 0, 0});
    scene->addObject(cube);

    bool p_open = true;

    while (!ENDER::Window::windowShouldClose() && p_open) {
        ENDER::Renderer::begin([&]() {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGui::Begin("DockSpace Demo", &p_open, window_flags);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Options")) {
                    if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "",
                                        (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) {
                        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
                    }
                    if (ImGui::MenuItem("Flag: NoDockingSplit", "",
                                        (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) {
                        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
                    }
                    if (ImGui::MenuItem("Flag: NoUndocking", "",
                                        (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) {
                        dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking;
                    }
                    if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) {
                        dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
                    }
                    if (ImGui::MenuItem("Flag: AutoHideTabBar", "",
                                        (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
                        dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
                    }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Close", NULL, false, &p_open != NULL))
                        p_open = false;
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            ImGui::Begin("Debug");
            ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
            if (ImGui::SliderFloat3("Direciton", glm::value_ptr(dir), -1, 1)) {
                directionalLight->setDirection(dir);
            }

            for (auto obj: scene->getObjects()) {
                if (obj->selected())
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                ImGui::Text("%s", obj->getName().c_str());
                if (obj->selected())
                    ImGui::PopStyleColor();
            }
            for (auto light: scene->getLights()) {
                if (light->type == ENDER::Light::LightType::PointLight)
                    ImGui::Text("PointLight");
            }
            ImGui::Begin("Viewport");



            float window_width = ImGui::GetContentRegionAvail().x;
            float window_height = ImGui::GetContentRegionAvail().y;

            if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
                          ImVec2 screen_pos = ImGui::GetCursorScreenPos();
                          auto mousePosition = ENDER::Window::getMousePosition();
                          spdlog::debug("{}, {}, {}", mousePosition.x - screen_pos.x, mousePosition.y - screen_pos.y, window_height - (mousePosition.y-screen_pos.y)-1);
                          auto pickedID =
                                  ENDER::Renderer::pickObjAt(mousePosition.x - screen_pos.x, (mousePosition.y-screen_pos.y), window_height);

                          for (auto object: scene->getObjects()) {
                              object->setSelected(object->getId() == pickedID);
                          }
                      }

            framebuffer->rescale(window_width, window_height);
            ENDER::Renderer::pickingResize(window_width, window_height);
            ENDER::Renderer::framebufferSizeCallback(window_width, window_height);
            // glViewport(0, 0, window_width, window_height);
            spdlog::debug("{}-{} {}-{}", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y, window_width, window_height);

            ImGui::Image(
                (ImTextureID) framebuffer->getTextureId(),
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            ImGui::End();

            ImGui::Begin("Picking");

            // and here we can add our created texture as image to ImGui
            // unfortunately we need to use the cast to void* or I didn't find another way tbh
            ImGui::Image(
                (ImTextureID) ENDER::Renderer::getPickingTextureID(),
                ImGui::GetWindowSize(),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            ImGui::End();


            ImGui::End();
            ImGui::End();
        });

        ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });

        ENDER::Renderer::renderScene(scene, framebuffer);

        ENDER::Renderer::end();

        camera->proccessInput();
    }

    return 0;
}
