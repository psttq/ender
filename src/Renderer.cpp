#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>


ENDER::Renderer::Renderer()
{
    createCubeVAO();
}

void ENDER::Renderer::init()
{
    spdlog::info("Inititing renderer.");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        throw;
    }

    ENDER::Window::setFramebufferSizeCallback(framebufferSizeCallback);

    instance()._projectMatrix = glm::mat4(1.0f);
    instance()._projectMatrix =
        glm::perspective(glm::radians(45.0f),
                         (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
    instance()._simpleShader = new Shader("../resources/simpleShader.vs", "../resources/simpleShader.fs");
    instance()._simpleShader->use();

    instance()._textureShader = new Shader("../resources/textureShader.vs", "../resources/textureShader.fs");
    instance()._textureShader->use();
    instance()._textureShader->setInt("texture1", 0);

    glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Window::instance().getNativeWindow(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void ENDER::Renderer::framebufferSizeCallback(int width, int height)
{
    instance()._projectMatrix =
       glm::perspective(glm::radians(45.0f),
                        (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
    glViewport(0, 0, width, height);
}

ENDER::Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete _textureShader;
    delete _simpleShader;
    delete cubeVAO;
    spdlog::info("Deallocation renderer.");
}

void ENDER::Renderer::renderObject(Object *object, Camera *camera)
{
    Shader *currentShader = object->getShader();

    if(currentShader == nullptr) {
        currentShader=instance()._simpleShader;
        if (object->getTexture() != nullptr)
        {
            currentShader=instance()._textureShader;
            object->getTexture()->setAsCurrent();
        }
    }

    currentShader->use();

    currentShader->setVec3("spotLight.position", camera->getPosition());
    currentShader->setVec3("spotLight.direction", camera->getFront());
    currentShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    currentShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    currentShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    currentShader->setFloat("spotLight.constant", 1.0f);
    currentShader->setFloat("spotLight.linear", 0.09f);
    currentShader->setFloat("spotLight.quadratic", 0.032f);
    currentShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    currentShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    currentShader->setBool("spotLight.toggled", camera->getSpotlightToggled());


    currentShader->setMat4("view", camera->getView());
    currentShader->setMat4("projection", _projectMatrix);

    auto objRotation = object->getRotation();

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, object->getPosition());
    model =
        glm::rotate(model, objRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model =
        glm::rotate(model, objRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model =
        glm::rotate(model, objRotation.z, glm::vec3(1.0f, 0.0f, 1.0f));

    model = glm::scale(model, object->getScale());

    currentShader->setMat4("model", model);

    object->getVertexArray()->bind();

    //FIXME: count?
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void ENDER::Renderer::setClearColor(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void ENDER::Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);
}

void ENDER::Renderer::swapBuffers()
{
    Window::swapBuffers();
}

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
    swapBuffers();
    Window::flash();
}

glm::mat4 ENDER::Renderer::getProjectMatrix() {
    return instance()._projectMatrix;
}

ENDER::Shader *ENDER::Renderer::shader() {
    return instance()._simpleShader;
}

void ENDER::Renderer::renderScene(Scene *scene) {
    for(const auto &obj: scene->getObjects()) {
        instance().renderObject(obj, scene->getCamera());
    }
}

void ENDER::Renderer::createCubeVAO()
{
    auto *cubeLayout = new BufferLayout(
        {{ENDER::LayoutObjectType::Float3}, {ENDER::LayoutObjectType::Float3},{ENDER::LayoutObjectType::Float2}});

    auto *cubeVBO = new VertexBuffer(cubeLayout);

    cubeVBO->setData(CUBE_VERTICES, sizeof(CUBE_VERTICES));
    cubeVAO = new VertexArray();
    cubeVAO->addVBO(cubeVBO);
}
