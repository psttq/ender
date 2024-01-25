#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    instance()._shader = new Shader("../resources/shader.vs", "../resources/shader.fs");
    instance()._shader->use();
    instance()._shader->setInt("texture1", 0);
    instance()._shader->setMat4(
        "projection",
        instance()._projectMatrix);



    glEnable(GL_DEPTH_TEST);
}

void ENDER::Renderer::framebufferSizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
}

ENDER::Renderer::~Renderer()
{
    delete _shader;
    delete cubeVAO;
    spdlog::info("Deallocation renderer.");
}

void ENDER::Renderer::renderObject(Object *object)
{
    instance()._shader->use();

    auto objRotation = object->getRotation();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, object->getPosition());
    model =
        glm::rotate(model, objRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model =
        glm::rotate(model, objRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model =
        glm::rotate(model, objRotation.z, glm::vec3(1.0f, 0.0f, 1.0f));

    instance()._shader->setMat4("model", model);

    object->getVertexArray()->bind();
    if (object->getTexture() != nullptr)
    {
        object->getTexture()->setAsCurrent();
    }
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

void ENDER::Renderer::renderScene(Scene *scene) {
    instance()._shader->setMat4("view", scene->calculateView());

    for(const auto &obj: scene->getObjects()) {
        instance().renderObject(obj);
    }
}

void ENDER::Renderer::createCubeVAO()
{
    auto *cubeLayout = new BufferLayout(
        {{ENDER::LayoutObjectType::Float3}, {ENDER::LayoutObjectType::Float2}});

    auto *cubeVBO = new VertexBuffer(cubeLayout);

    cubeVBO->setData(CUBE_VERTICES, sizeof(CUBE_VERTICES));
    cubeVAO = new VertexArray();
    cubeVAO->addVBO(cubeVBO);
}
