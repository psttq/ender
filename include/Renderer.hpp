#pragma once
#include <Object.hpp>
#include <PickingTexture.hpp>
#include <Scene.hpp>
#include <Shader.hpp>
#include <Window.hpp>

namespace ENDER {

static const int MAX_POINT_LIGHTS_NUMBER = 100;

class Renderer {
  Shader *_simpleShader;
  Shader *_textureShader;
  Shader *_gridShader;
  Shader *_pickingEffect;

  glm::mat4 _projectMatrix;

  PickingTexture *_pickingTexture;

  VertexArray *cubeVAO;
  VertexArray *gridVAO;

  Renderer();
  ~Renderer();

  void createCubeVAO();
  void createGridVAO();
  void renderObject(Object *object, Scene *scene);
  void renderObject(Object *object, Scene *scene, Shader *shader);
  void renderObjectToPicking(Object *object, Scene *scene);

 public:
  static Renderer &instance() {
    static Renderer _instance;
    return _instance;
  }

  static void init();

  static void setClearColor(const glm::vec4 &color);
  // static void swapBuffers(const glm::vec4 &color);

  static void clear();
  static void clearPicking();

  static void swapBuffers();

  static void begin(std::function<void()> imguiDrawCallback);

  static void end();

  static glm::mat4 getProjectMatrix();

  static unsigned int getPickingTextureID();

  static Shader *shader();

  static void renderScene(Scene *scene);

  static unsigned int pickObjAt(unsigned int x, unsigned int y);

  static void framebufferSizeCallback(int width, int height);

  static Shader *getGridShader() { return instance()._gridShader; }

  static VertexArray *getCubeVAO() { return instance().cubeVAO; }
  static VertexArray *getGridVAO() { return instance().gridVAO; }
};

}  // namespace ENDER
