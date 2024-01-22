#include <Ender.hpp>

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{

  ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

  ENDER::Renderer::init();

  spdlog::set_level(spdlog::level::debug);

  glm::vec3 cubePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f), glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f), glm::vec3(-1.3f, 1.0f, -1.5f)};

  // auto layout = new ENDER::BufferLayout(
  //     {{ENDER::LayoutObjectType::Float3}, {ENDER::LayoutObjectType::Float2}});

  // auto vertexBuffer = new ENDER::VertexBuffer(layout);

  // vertexBuffer->setData(vertices, sizeof(vertices));

  // auto vao = new ENDER::VertexArray();

  // vao->addVBO(vertexBuffer);

  // load and create a texture
  auto texture1 = new ENDER::Texture();
  texture1->loadFromFile("../resources/textures/container.jpg", GL_RGB);

  auto texture2 = new ENDER::Texture();
  texture2->loadFromFile("../resources/textures/awesomeface.png", GL_RGBA);

  auto cubeObject = ENDER::Object::createCube("cube");
  cubeObject->setTexture(texture1);
  cubeObject->setPosition(glm::vec3(1.5f, 0.2f, -1.5f));

  auto cubeObject2 = ENDER::Object::createCube("cube2");
  cubeObject2->setTexture(texture2);
  cubeObject2->setPosition(glm::vec3(-1.5f, 0.2f, -1.5f));

  ENDER::Renderer::setClearColor({0.2f, 0.3f, 0.3f, 1.0f});

  float angle = 0;

  double lastFrame = glfwGetTime();
  while (!ENDER::Window::windowShouldClose())
  {
    double currentTime = glfwGetTime();
    double deltaTime = (currentTime - lastFrame);
    lastFrame = currentTime;

    angle += 1.0f * deltaTime;
    cubeObject->setRotation(glm::vec3(0, angle, 0));
    // spdlog::debug(" fps={}", 1 / deltaTime);

    processInput(ENDER::Window::instance().getNativeWindow());

    ENDER::Renderer::clear();

    ENDER::Renderer::renderObject(cubeObject);
    ENDER::Renderer::renderObject(cubeObject2);

    ENDER::Renderer::swapBuffers();
    ENDER::Window::pollEvents();
  }

  // delete cubeObject;
  // delete cubeObject2;

  return 0;
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}