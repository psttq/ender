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
  Shader *_debugSquareShader;

  glm::mat4 _projectMatrix;

  PickingTexture *_pickingTexture;

  VertexArray *cubeVAO;
  VertexArray *gridVAO;
  VertexArray *debugSquareVAO;

  Renderer();
  ~Renderer();

  void createCubeVAO();
  void createGridVAO();
  void createDebugSquareVAO();

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

  static void renderDebugTexture(unsigned int textureID);
  static void renderDebugTexture(Texture *texture);

  static Shader *shader();

  static void renderScene(Scene *scene);

  static unsigned int pickObjAt(unsigned int x, unsigned int y);

  static void framebufferSizeCallback(int width, int height);

  static Shader *getGridShader() { return instance()._gridShader; }

  static VertexArray *getCubeVAO() { return instance().cubeVAO; }
  static VertexArray *getGridVAO() { return instance().gridVAO; }
};

static float debugSquareVertices[] = {
    // positions       // texture coords
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // top right
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // top left
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // top right
  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
};
static unsigned int debugSquareIndices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

} // namespace ENDER
