#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include "DirectionalLight.hpp"
#include "PickingTexture.hpp"
#include "PointLight.hpp"

ENDER::Renderer::Renderer() {}

void ENDER::Renderer::init() {
  spdlog::info("Inititing renderer.");

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    spdlog::error("Failed to initialize GLAD");
    throw;
  }

  ENDER::Window::setFramebufferSizeCallback(framebufferSizeCallback);

  instance()._projectMatrix = glm::mat4(1.0f);
  instance()._projectMatrix = glm::perspective(
      glm::radians(45.0f),
      (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
  instance()._simpleShader = new Shader("../resources/simpleShader.vs",
                                        "../resources/simpleShader.fs");
  instance()._simpleShader->use();

  instance()._textureShader = new Shader("../resources/textureShader.vs",
                                         "../resources/textureShader.fs");
  instance()._textureShader->use();
  instance()._textureShader->setInt("texture1", 0);

  instance()._gridShader =
      new Shader("../resources/gridShader.vs", "../resources/gridShader.fs");

  glEnable(GL_DEPTH_TEST);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(Window::instance().getNativeWindow(), true);
  // Second param install_callback=true will install GLFW callbacks and chain to
  // existing ones.
  ImGui_ImplOpenGL3_Init();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  instance().createCubeVAO();
  instance().createGridVAO();

  auto windowSize = Window::getSize();

  instance()._pickingTexture = new PickingTexture();
  instance()._pickingTexture->init(windowSize.x, windowSize.y);
  instance()._pickingEffect =
      new ENDER::Shader("../resources/picking.vs", "../resources/picking.fs");
}

void ENDER::Renderer::framebufferSizeCallback(int width, int height) {
  instance()._projectMatrix = glm::perspective(
      glm::radians(45.0f),
      (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
  instance()._pickingTexture->updateTextureSize(width, height);
  glViewport(0, 0, width, height);
}

ENDER::Renderer::~Renderer() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  delete _textureShader;
  delete _simpleShader;
  delete _gridShader;

  delete _pickingTexture;

  delete cubeVAO;
  delete gridVAO;
  spdlog::info("Deallocation renderer.");
}

void ENDER::Renderer::renderObject(Object *object, Scene *scene) {
  auto camera = scene->getCamera();

  Shader *currentShader = object->getShader();

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

  auto objRotation = object->getRotation();

  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, object->getPosition());
  model = glm::rotate(model, objRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, objRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, objRotation.z, glm::vec3(1.0f, 0.0f, 1.0f));

  model = glm::scale(model, object->getScale());

  currentShader->setMat4("model", model);

  unsigned int pointLightsCount = 0;

  for (auto light : scene->getLights()) {
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
    } break;
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
    } break;
    default:
      spdlog::error("ENDER::Renderer::renderObject: unknown type of light");
      continue;
    }
  }

  currentShader->setInt("pointLightsCount", pointLightsCount);

  currentShader->setBool("selected", object->selected());

  object->getVertexArray()->bind();

  // FIXME: count?
  glDrawArrays(GL_TRIANGLES, 0, 36);
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
  swapBuffers();
  Window::flash();
}

glm::mat4 ENDER::Renderer::getProjectMatrix() {
  return instance()._projectMatrix;
}

ENDER::Shader *ENDER::Renderer::shader() { return instance()._simpleShader; }

void ENDER::Renderer::renderScene(Scene *scene) {
  for (const auto &obj : scene->getObjects()) {
    /* RENDERING TO PICKING TEXTURE */
    instance().renderObjectToPicking(obj, scene);
    /* RENDERING TO DEFAULT FRAMEBUFFER */
    instance().renderObject(obj, scene);
  }
}

void ENDER::Renderer::renderObjectToPicking(Object *object, Scene *scene) {
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
  auto *cubeLayout = new BufferLayout({{ENDER::LayoutObjectType::Float3},
                                       {ENDER::LayoutObjectType::Float3},
                                       {ENDER::LayoutObjectType::Float2}});

  auto *cubeVBO = new VertexBuffer(cubeLayout);

  cubeVBO->setData(CUBE_VERTICES, sizeof(CUBE_VERTICES));
  cubeVAO = new VertexArray();
  cubeVAO->addVBO(cubeVBO);
}

void ENDER::Renderer::createGridVAO() {
  auto gridLayout = new ENDER::BufferLayout({ENDER::LayoutObjectType::Float3});
  auto gridVBO = new ENDER::VertexBuffer(gridLayout);
  gridVBO->setData(GRID_VERTICES, sizeof(GRID_VERTICES));

  gridVAO = new ENDER::VertexArray();
  gridVAO->addVBO(gridVBO);

  _gridShader->use();

  float near = 0.01;
  float far = 1.5;

  _gridShader->setFloat("near", near);
  _gridShader->setFloat("far", far);
}

void ENDER::Renderer::renderObject(ENDER::Object *object, ENDER::Scene *scene,
                                   ENDER::Shader *shader) {
  auto camera = scene->getCamera();
  shader->use();
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

  // FIXME: count?
  glDrawArrays(GL_TRIANGLES, 0, 36);
}
