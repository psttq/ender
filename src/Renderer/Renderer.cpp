#include <../../3rd/imgui/backends/imgui_impl_glfw.h>
#include <../../3rd/imgui/backends/imgui_impl_opengl3.h>
#include <../../3rd/imgui/imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <../../3rd/glm/glm/glm.hpp>
#include <../../3rd/glm/glm/gtc/matrix_transform.hpp>
#include <../../3rd/glm/glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <../../include/Renderer/Renderer.hpp>
#include <memory>

#include "../../include/Renderer/BufferLayout.hpp"
#include "../../include/Renderer/DirectionalLight.hpp"
#include "../../include/Renderer/Framebuffer.hpp"
#include "../../include/Renderer/PickingTexture.hpp"
#include "../../include/Renderer/PointLight.hpp"
#include "../../include/Renderer/VertexBuffer.hpp"


ENDER::Renderer::Renderer() {}

void ENDER::Renderer::init() {
    spdlog::info("Inititing renderer.");

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        spdlog::error("Failed to initialize GLAD");
        throw;
    }

    ENDER::Window::setFramebufferSizeCallback(framebufferSizeCallback);

    instance()._projectMatrix = glm::mat4(1.0f);
    instance()._projectMatrix = glm::perspective(
            glm::radians(45.0f),
            (float) Window::getWidth() / (float) Window::getHeight(), 0.1f, 100.0f);
    instance()._simpleShader =
            Shader::create("../resources/shaders/simpleShader.vs",
                           "../resources/shaders/simpleShader.fs",
                           "../resources/shaders/normalsShader.gs");

    instance()._simpleShaderLine =
            Shader::create("../resources/shaders/simpleShaderLine.vs",
                           "../resources/shaders/simpleShaderLine.fs");

    instance()._textureShader =
            Shader::create("../resources/shaders/textureShader.vs",
                           "../resources/shaders/textureShader.fs");
    instance()._textureShader->use();
    instance()._textureShader->setInt("texture1", 0);

    instance()._debugNormalsShader =
            Shader::create("../resources/shaders/normalsDebugShader.vs",
                           "../resources/shaders/normalsDebugShader.fs",
                           "../resources/shaders/normalsDebugShader.gs");

    instance()._gridShader =
            std::make_shared<Shader>("../resources/shaders/gridShader.vs",
                                     "../resources/shaders/gridShader.fs");

    instance()._debugSquareShader =
            Shader::create("../resources/shaders/debugFramebuffer.vs",
                           "../resources/shaders/debugFramebuffer.fs");

    glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Window::instance().getNativeWindow(), true);
    // Second param install_callback=true will install GLFW callbacks and chain to
    // existing ones.
    ImGui_ImplOpenGL3_Init();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    instance().createCubeVAO();
    instance().createGridVAO();
    instance().createDebugSquareVAO();
    instance().createCircleVAO();

    auto windowSize = Window::getSize();

    instance()._pickingTexture = PickingTexture::create();
    instance()._pickingTexture->init(windowSize.x, windowSize.y);
    instance()._pickingEffect = Shader::create("../resources/shaders/picking.vs",
                                               "../resources/shaders/picking.fs");
    glLineWidth(LINE_WIDTH);
}

unsigned int ENDER::Renderer::getPickingTextureID() {
    return instance()._pickingTexture->getTextureID();
}

void ENDER::Renderer::framebufferSizeCallback(int width, int height) {
    glViewport(0, 0, width, height);
}

void ENDER::Renderer::pickingResize(float width, float height) {
    instance()._pickingTexture->updateTextureSize(width, height);
}

ENDER::Renderer::~Renderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    spdlog::info("Deallocation renderer.");
}

void ENDER::Renderer::_configureSpotLight(sptr<Shader> shader,
                                          sptr<Camera> camera) {
    shader->setVec3("spotLight.position", camera->getPosition());
    shader->setVec3("spotLight.direction", camera->getFront());
    shader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLight.constant", 1.0f);
    shader->setFloat("spotLight.linear", 0.09f);
    shader->setFloat("spotLight.quadratic", 0.032f);
    shader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    shader->setBool("spotLight.toggled", camera->getSpotlightToggled());
}

void ENDER::Renderer::_configurePointLight(sptr<Shader> shader,
                                           const std::string &pointLightLabel,
                                           PointLight *pointLight) {
    shader->setVec3(pointLightLabel + ".position", pointLight->position());
    shader->setVec3(pointLightLabel + ".ambient", pointLight->ambient());
    shader->setVec3(pointLightLabel + ".diffuse", pointLight->diffuse());
    shader->setVec3(pointLightLabel + ".specular", pointLight->specular());
    shader->setFloat(pointLightLabel + ".constant", pointLight->constant());
    shader->setFloat(pointLightLabel + ".linear", pointLight->linear());
    shader->setFloat(pointLightLabel + ".quadratic", pointLight->quadratic());
}

void ENDER::Renderer::_configureDirectionLight(
        sptr<Shader> shader, DirectionalLight *directionalLight) {
    shader->setBool("dirLight.enabled", true);
    shader->setVec3("dirLight.direction", directionalLight->direction());
    shader->setVec3("dirLight.ambient", directionalLight->ambient());
    shader->setVec3("dirLight.diffuse", directionalLight->diffuse());
    shader->setVec3("dirLight.specular", directionalLight->specular());
}

void ENDER::Renderer::_configureLight(sptr<Shader> shader, sptr<Scene> scene) {
    unsigned int pointLightsCount = 0;

    for (auto light: scene->getLights()) {
        switch (light->type) {
            case Light::LightType::PointLight: {
                if (pointLightsCount == MAX_POINT_LIGHTS_NUMBER)
                    continue;

                auto pointLight = dynamic_cast<PointLight *>(light);
                if (pointLight == nullptr) {
                    spdlog::error("ENDER::Renderer::renderObject: something went wrong "
                                  "when casting light");
                    continue;
                }
                std::string pointLightLabel =
                        std::string("pointLights[") + std::to_string(pointLightsCount) + "]";

                // spdlog::debug("ENDER::Renderer::renderObject: adding {}",
                // pointLightLabel);
                _configurePointLight(shader, pointLightLabel, pointLight);

                pointLightsCount++;
            }
                break;
            case Light::LightType::DirectionalLight: {
                auto directionalLight = dynamic_cast<DirectionalLight *>(light);
                if (directionalLight == nullptr) {
                    spdlog::error("ENDER::Renderer::renderObject: something went wrong "
                                  "when casting light");
                    continue;
                }
                _configureDirectionLight(shader, directionalLight);
            }
                break;
            default:
                spdlog::error("ENDER::Renderer::renderObject: unknown type of light");
                continue;
        }
    }

    shader->setInt("pointLightsCount", pointLightsCount);
}

void ENDER::Renderer::renderObject(sptr<Object> object, sptr<Scene> scene) {
    auto camera = scene->getCamera();

    auto currentShader = object->getShader();

    if (currentShader == nullptr) {
        if (object->type == Object::ObjectType::Surface) {
            currentShader = instance()._simpleShader;
            currentShader->use();
            currentShader->setVec3("material.diffuse", object->material.diffuse);
            currentShader->setVec3("material.ambient", object->material.ambient);
            if (object->getTexture() != nullptr) {
                currentShader = instance()._textureShader;
                object->getTexture()->setAsCurrent();
                currentShader->use();
                currentShader->setInt("material.diffuse", 0);
            }
        } else if (object->type == Object::ObjectType::Line) {
            currentShader = instance()._simpleShaderLine;
            currentShader->use();
            currentShader->setVec3("material.diffuse", object->material.diffuse);
            currentShader->setVec3("material.ambient", object->material.ambient);
        }
    } else
        currentShader->use();

    currentShader->setVec3("viewPos", camera->getPosition());

    currentShader->setVec3("material.specular", object->material.specular);
    currentShader->setFloat("material.shininess", object->material.shininess);

    _configureSpotLight(currentShader, camera);

    currentShader->setMat4("view", camera->getView());
    currentShader->setMat4("projection", camera->getProjection());

    currentShader->setFloat("time", Window::currentTime());

    auto model = object->getTransform();

    currentShader->setMat4("model", model);

    _configureLight(currentShader, scene);

    currentShader->setBool("selected", object->selected());

    object->getVertexArray()->bind();

    auto drawType = GL_TRIANGLES;
    switch (_drawType) {
        case DrawType::Triangles: {
            drawType = GL_TRIANGLES;
        }
            break;
        case DrawType::Lines: {
            drawType = GL_LINES;
        }
            break;
        default:
            spdlog::error("Unreachable");
    }

    if (object->type == Object::ObjectType::Line)
        drawType = GL_LINE_STRIP;

    if (object->getVertexArray()->isIndexBuffer())
        glDrawElements(drawType, object->getVertexArray()->indexCount(),
                       GL_UNSIGNED_INT, 0);
    else {
        glDrawArrays(drawType, 0, object->getVertexArray()->verticesCount());
    }
}

void ENDER::Renderer::setClearColor(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void ENDER::Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ENDER::Renderer::clearPicking() {
    instance()._pickingTexture->enableWriting();
    clear();
    instance()._pickingTexture->disableWriting();
}

void ENDER::Renderer::swapBuffers() { Window::swapBuffers(); }

void ENDER::Renderer::begin(std::function<void()> imguiDrawCallback) {
    Window::pollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    imguiDrawCallback();
}

void ENDER::Renderer::end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Update and Render additional Platform Windows
    // ImGui::UpdatePlatformWindows();
    // ImGui::RenderPlatformWindowsDefault();

    swapBuffers();
    Window::flash();
}

void ENDER::Renderer::setDrawType(DrawType drawType) {
    instance()._drawType = drawType;
}

glm::mat4 ENDER::Renderer::getProjectMatrix() {
    return instance()._projectMatrix;
}

sptr<ENDER::Shader> ENDER::Renderer::shader() {
    return instance()._simpleShader;
}

void ENDER::Renderer::renderScene(sptr<Scene> scene,
                                  sptr<Framebuffer> framebuffer) {
    framebuffer->clear();

    framebuffer->bind();
    /* RENDERING TO FRAMEBUFFER */
    for (const auto &obj: scene->getObjects()) {
        instance().renderObject(obj, scene);
        if (instance()._renderNormals)
            instance().renderObject(obj, scene, instance()._debugNormalsShader);
        if(obj->type == Object::ObjectType::Multi)
            instance().renderObject(obj->getChildObject(), scene);

    }
    framebuffer->unbind();

    /* RENDERING TO PICKING TEXTURE */
    for (const auto &obj: scene->getObjects()) {
        if (obj->isSelectable)
            instance().renderObjectToPicking(obj, scene,
                                             framebuffer->getPickingTexture());
    }
}

void ENDER::Renderer::renderScene(sptr<Scene> scene) {
    clear();
    /* RENDERING TO DEFAULT FRAMEBUFFER */
    for (const auto &obj: scene->getObjects()) {
        instance().renderObject(obj, scene);
        if (instance()._renderNormals)
            instance().renderObject(obj, scene, instance()._debugNormalsShader);
        if(obj->type == Object::ObjectType::Multi)
            instance().renderObject(obj->getChildObject(), scene);
    }
    /* RENDERING TO PICKING TEXTURE */
    clearPicking();
    for (const auto &obj: scene->getObjects()) {
        if (obj->isSelectable)
            instance().renderObjectToPicking(obj, scene, instance()._pickingTexture);
    }
}

void ENDER::Renderer::renderObjectToPicking(
        sptr<Object> object, sptr<Scene> scene,
        sptr<PickingTexture> pickingTexture) {
    pickingTexture->enableWriting();
    instance()._pickingEffect->use();
    instance()._pickingEffect->setInt("gObjectIndex", object->getId());
    instance()._pickingEffect->setInt("gDrawIndex", 0); // TODO: impl
    instance().renderObject(object, scene, instance()._pickingEffect);
    pickingTexture->disableWriting();
}

unsigned int ENDER::Renderer::pickObjAt(unsigned int x, unsigned int y,
                                        unsigned int window_height) {
    return instance()
            ._pickingTexture->readPixel(x, window_height - y - 1)
            .objectID;
}

void ENDER::Renderer::createCircleVAO() {
    std::vector<float> vertices;
    float dangle = 2 * glm::pi<float>() / (CIRCLE_VERTICES_COUNT - 1);
    for (auto i = 0; i < CIRCLE_VERTICES_COUNT - 1; i++) {
        float angle = dangle * i;
        vertices.insert(vertices.end(), {glm::sin(angle) * CIRCLE_RADIUS, 0.01,
                                         glm::cos(angle) * CIRCLE_RADIUS});
        vertices.insert(vertices.end(), {0, 0.01, 0});
        angle = dangle * (i + 1);
        vertices.insert(vertices.end(), {glm::sin(angle) * CIRCLE_RADIUS, 0.01,
                                         glm::cos(angle) * CIRCLE_RADIUS});
    }

    auto layout =
            uptr<BufferLayout>(new BufferLayout({{LayoutObjectType::Float3}}));
    auto vbo = std::make_unique<VertexBuffer>(std::move(layout));
    vbo->setData(&vertices[0], vertices.size() * sizeof(float));
    circleVAO = std::make_shared<VertexArray>();
    circleVAO->addVBO(std::move(vbo));
}

void ENDER::Renderer::createCubeVAO() {
    auto cubeLayout =
            uptr<BufferLayout>(new BufferLayout({{ENDER::LayoutObjectType::Float3},
                                                 {ENDER::LayoutObjectType::Float3},
                                                 {ENDER::LayoutObjectType::Float2}}));

    auto cubeVBO = std::make_unique<VertexBuffer>(std::move(cubeLayout));
    cubeVBO->setData(CUBE_VERTICES, sizeof(CUBE_VERTICES));

    cubeVAO = std::make_shared<VertexArray>();
    cubeVAO->addVBO(std::move(cubeVBO));
}

void ENDER::Renderer::createGridVAO() {
    auto gridLayout = uptr<BufferLayout>(
            new ENDER::BufferLayout({ENDER::LayoutObjectType::Float3}));
    auto gridVBO = std::make_unique<VertexBuffer>(std::move(gridLayout));
    gridVBO->setData(GRID_VERTICES, sizeof(GRID_VERTICES));

    gridVAO = std::make_shared<ENDER::VertexArray>();
    gridVAO->addVBO(std::move(gridVBO));

    _gridShader->use();

    float near = 0.01;
    float far = 1.5;

    _gridShader->setFloat("near", near);
    _gridShader->setFloat("far", far);
}

void ENDER::Renderer::createDebugSquareVAO() {
    spdlog::debug("Creating debug VAO");
    auto layout = uptr<BufferLayout>(new BufferLayout(
            {{ENDER::LayoutObjectType::Float3},
             {ENDER::LayoutObjectType::Float2}}));
    auto vbo = std::make_unique<VertexBuffer>(std::move(layout));
    vbo->setData(debugSquareVertices, sizeof(debugSquareVertices));

    debugSquareVAO = std::make_shared<VertexArray>();
    debugSquareVAO->addVBO(std::move(vbo));
}

void ENDER::Renderer::renderDebugTexture(unsigned int textureID) {
    instance()._debugSquareShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    instance().debugSquareVAO->bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ENDER::Renderer::renderDebugTexture(Texture *texture) {
    texture->setAsCurrent();
    instance()._debugSquareShader->use();
    instance().debugSquareVAO->bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ENDER::Renderer::setRenderNormals(bool value) {
    instance()._renderNormals = value;
}

bool ENDER::Renderer::isRenderingNormals() { return instance()._renderNormals; }

void ENDER::Renderer::renderObject(sptr<Object> object, sptr<Scene> scene,
                                   sptr<Shader> shader) {
    auto camera = scene->getCamera();
    shader->use();

    shader->setFloat("time", Window::currentTime());
    shader->setMat4("view", camera->getView());
    shader->setMat4("projection", camera->getProjection());

    auto objRotation = object->getRotation();

    auto model = object->getTransform();

    shader->setMat4("model", model);
    object->getVertexArray()->bind();

    if (object->getVertexArray()->isIndexBuffer())
        glDrawElements(GL_TRIANGLES, object->getVertexArray()->indexCount(),
                       GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, object->getVertexArray()->verticesCount());
}

void ENDER::Renderer::renderObject(sptr<Object> object, sptr<Scene> scene, sptr<Framebuffer> framebuffer) {
    framebuffer->bind();
    /* RENDERING TO FRAMEBUFFER */
    instance().renderObject(object, scene);
    framebuffer->unbind();
    if (object->isSelectable)
        instance().renderObjectToPicking(object, scene, framebuffer->getPickingTexture());
}
