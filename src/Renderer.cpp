#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "BufferLayout.hpp"
#include "DirectionalLight.hpp"
#include "Framebuffer.hpp"
#include "PickingTexture.hpp"
#include "PointLight.hpp"
#include "VertexBuffer.hpp"

ENDER::Renderer::Renderer() {
}

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
    instance()._simpleShader = Shader::create("../resources/shaders/simpleShader.vs",
                                              "../resources/shaders/simpleShader.fs"
                                              );
    instance()._simpleShader->use();

    instance()._textureShader = Shader::create("../resources/shaders/textureShader.vs",
                                               "../resources/shaders/textureShader.fs");
    instance()._textureShader->use();
    instance()._textureShader->setInt("texture1", 0);

    instance()._debugNormalsShader = Shader::create("../resources/shaders/normalsDebugShader.vs",
                                                    "../resources/shaders/normalsDebugShader.fs",
                                                    "../resources/shaders/normalsDebugShader.gs");

    instance()._gridShader =
            std::make_shared<Shader>("../resources/shaders/gridShader.vs", "../resources/shaders/gridShader.fs");

    instance()._debugSquareShader = Shader::create(
        "../resources/shaders/debugFramebuffer.vs", "../resources/shaders/debugFramebuffer.fs");

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

    auto windowSize = Window::getSize();

    instance()._pickingTexture = new PickingTexture();
    instance()._pickingTexture->init(windowSize.x, windowSize.y);
    instance()._pickingEffect = Shader::create("../resources/shaders/picking.vs", "../resources/shaders/picking.fs");
}

unsigned int ENDER::Renderer::getPickingTextureID() {
    return instance()._pickingTexture->getTextureID();
}

void ENDER::Renderer::framebufferSizeCallback(int width, int height) {
    instance()._projectMatrix = glm::perspective(
        glm::radians(45.0f),
        (float) width / (float) height, 0.1f, 100.0f);
    // instance()._pickingTexture->updateTextureSize(width, height);

    spdlog::debug("window size: {}, {}", width, height);
    glViewport(0, 0, width, height);
}

void ENDER::Renderer::pickingResize(float width, float height) {
    instance()._pickingTexture->updateTextureSize(width, height);
}

ENDER::Renderer::~Renderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete _pickingTexture;


    spdlog::info("Deallocation renderer.");
}

void ENDER::Renderer::renderObject(sptr<Object> object, sptr<Scene> scene) {
    auto camera = scene->getCamera();

    auto currentShader = object->getShader();

    if (currentShader == nullptr) {
        currentShader = instance()._simpleShader;
        currentShader->use();
        currentShader->setVec3("material.diffuse", {1.0f, 0.5f, 0.31f});
        currentShader->setVec3("material.ambient", {1.0f, 0.5f, 0.31f});
        if (object->getTexture() != nullptr) {
            currentShader = instance()._textureShader;
            object->getTexture()->setAsCurrent();
            currentShader->use();
            currentShader->setInt("material.diffuse", 0);
        }
    } else
        currentShader->use();

    currentShader->setVec3("viewPos", camera->getPosition());

    currentShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    currentShader->setFloat("material.shininess", 64.0f);

    currentShader->setVec3("spotLight.position", camera->getPosition());
    currentShader->setVec3("spotLight.direction", camera->getFront());
    currentShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    currentShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    currentShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    currentShader->setFloat("spotLight.constant", 1.0f);
    currentShader->setFloat("spotLight.linear", 0.09f);
    currentShader->setFloat("spotLight.quadratic", 0.032f);
    currentShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    currentShader->setFloat("spotLight.outerCutOff",
                            glm::cos(glm::radians(15.0f)));

    currentShader->setBool("spotLight.toggled", camera->getSpotlightToggled());

    currentShader->setMat4("view", camera->getView());
    currentShader->setMat4("projection", _projectMatrix);

    currentShader->setFloat("time", Window::currentTime());

    auto objRotation = object->getRotation();

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, object->getPosition());
    model = glm::rotate(model, objRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, objRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, objRotation.z, glm::vec3(1.0f, 0.0f, 1.0f));

    model = glm::scale(model, object->getScale());

    currentShader->setMat4("model", model);

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

                currentShader->setVec3(pointLightLabel + ".position",
                                       pointLight->position());
                currentShader->setVec3(pointLightLabel + ".ambient",
                                       pointLight->ambient());
                currentShader->setVec3(pointLightLabel + ".diffuse",
                                       pointLight->diffuse());
                currentShader->setVec3(pointLightLabel + ".specular",
                                       pointLight->specular());
                currentShader->setFloat(pointLightLabel + ".constant",
                                        pointLight->constant());
                currentShader->setFloat(pointLightLabel + ".linear",
                                        pointLight->linear());
                currentShader->setFloat(pointLightLabel + ".quadratic",
                                        pointLight->quadratic());

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
                currentShader->setBool("dirLight.enabled", true);
                currentShader->setVec3("dirLight.direction",
                                       directionalLight->direction());
                currentShader->setVec3("dirLight.ambient", directionalLight->ambient());
                currentShader->setVec3("dirLight.diffuse", directionalLight->diffuse());
                currentShader->setVec3("dirLight.specular", directionalLight->specular());
            }
            break;
            default:
                spdlog::error("ENDER::Renderer::renderObject: unknown type of light");
                continue;
        }
    }

    currentShader->setInt("pointLightsCount", pointLightsCount);

    currentShader->setBool("selected", object->selected());
    object->getVertexArray()->bind();

    auto drawType = GL_TRIANGLES;
    switch (_drawType) {
        case DrawType::Triangles: {
            drawType = GL_TRIANGLES;
        }
        break;
        case DrawType::Lines: {
            drawType = GL_LINE_STRIP;
        }
        break;
        default:
            spdlog::error("Unreachable");
    }

    if (object->getVertexArray()->isIndexBuffer())
        glDrawElements(drawType, object->getVertexArray()->indexCount(),GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(drawType, 0, 36);
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

sptr<ENDER::Shader> ENDER::Renderer::shader() { return instance()._simpleShader; }

void ENDER::Renderer::renderScene(sptr<Scene> scene, sptr<Framebuffer> framebuffer) {
    framebuffer->bind();
    clear();
    /* RENDERING TO FRAMEBUFFER */
    for (const auto &obj: scene->getObjects()) {
        instance().renderObject(obj, scene);
        instance().renderObject(obj, scene, instance()._debugNormalsShader);
    }
    framebuffer->unbind();

    /* RENDERING TO PICKING TEXTURE */
    clearPicking();
    for (const auto &obj: scene->getObjects()) {
        instance().renderObjectToPicking(obj, scene);
    }

}

void ENDER::Renderer::renderObjectToPicking(sptr<Object> object, sptr<Scene> scene) {
    instance()._pickingTexture->enableWriting();
    instance()._pickingEffect->use();
    instance()._pickingEffect->setInt("gObjectIndex", object->getId());
    instance()._pickingEffect->setInt("gDrawIndex", 0); // TODO: impl
    instance().renderObject(object, scene, instance()._pickingEffect);
    instance()._pickingTexture->disableWriting();
}

unsigned int ENDER::Renderer::pickObjAt(unsigned int x, unsigned int y) {
    auto height = Window::getHeight();
    return instance()._pickingTexture->readPixel(x, height - y - 1).objectID;
}

void ENDER::Renderer::createCubeVAO() {
    auto cubeLayout = uptr<BufferLayout>(new BufferLayout({
        {ENDER::LayoutObjectType::Float3},
        {ENDER::LayoutObjectType::Float3},
        {ENDER::LayoutObjectType::Float2}
    }));

    auto cubeVBO = std::make_unique<VertexBuffer>(std::move(cubeLayout));

    cubeVBO->setData(CUBE_VERTICES, sizeof(CUBE_VERTICES));
    cubeVAO = std::make_shared<VertexArray>();
    cubeVAO->addVBO(std::move(cubeVBO));
}

void ENDER::Renderer::createGridVAO() {
    auto gridLayout = uptr<BufferLayout>(new ENDER::BufferLayout({ENDER::LayoutObjectType::Float3}));
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
        {{ENDER::LayoutObjectType::Float3}, {ENDER::LayoutObjectType::Float2}}));
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

void ENDER::Renderer::renderObject(sptr<Object> object, sptr<Scene> scene,
                                   sptr<Shader> shader) {
    auto camera = scene->getCamera();
    shader->use();

    shader->setFloat("time", Window::currentTime());
    shader->setMat4("view", camera->getView());
    shader->setMat4("projection", _projectMatrix);

    auto objRotation = object->getRotation();

    auto model = glm::mat4(1.0f);

    model = glm::translate(model, object->getPosition());
    model = glm::rotate(model, objRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, objRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, objRotation.z, glm::vec3(1.0f, 0.0f, 1.0f));
    model = glm::scale(model, object->getScale());
    shader->setMat4("model", model);
    object->getVertexArray()->bind();

    if (object->getVertexArray()->isIndexBuffer())
        glDrawElements(GL_TRIANGLES, object->getVertexArray()->indexCount(),GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, 36); // FIXME: count?
}
