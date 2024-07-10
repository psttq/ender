#pragma once
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "VertexArray.hpp"
#include <Object.hpp>
#include <Renderer/PickingTexture.hpp>
#include <Scene.hpp>
#include <Shader.hpp>
#include <Window.hpp>

#include "Framebuffer.hpp"

namespace ENDER {

static const int MAX_POINT_LIGHTS_NUMBER = 100;

#define CIRCLE_VERTICES_COUNT 20
#define CIRCLE_RADIUS 0.020f

#define LINE_WIDTH 2

class Renderer {
public:
  enum class DrawType { Triangles, Lines };

private:
  DrawType _drawType = DrawType::Triangles;

  sptr<Shader> _simpleShader;
  sptr<Shader> _textureShader;
  sptr<Shader> _gridShader;
  sptr<Shader> _pickingEffect;
  sptr<Shader> _debugSquareShader;
  sptr<Shader> _debugNormalsShader;
  sptr<Shader> _simpleShaderLine;

  glm::mat4 _projectMatrix;

  sptr<PickingTexture> _pickingTexture;

  sptr<VertexArray> cubeVAO;
  sptr<VertexArray> gridVAO;
  sptr<VertexArray> debugSquareVAO;
  sptr<VertexArray> circleVAO;

  Renderer();
  ~Renderer();

  void createCubeVAO();
  void createGridVAO();
  void createDebugSquareVAO();
  void createCircleVAO();

  void renderObject(sptr<Object> object, sptr<Scene> scene, std::optional<glm::mat4> model = std::nullopt);
  void renderObject(sptr<Object> object, sptr<Scene> scene,
                    sptr<Shader> shader,  std::optional<glm::mat4> model = std::nullopt);
  void renderObjectToPicking(sptr<Object> object, sptr<Scene> scene,
                             sptr<PickingTexture> pickingTexture);

  void _configureSpotLight(sptr<Shader> shader, sptr<Camera> camera);
  void _configurePointLight(sptr<Shader> shader,
                            const std::string &pointLightLabel,
                            PointLight *pointLight);
  void _configureDirectionLight(sptr<Shader> shader,
                                DirectionalLight *directionalLight);

  void _configureLight(sptr<Shader> shader, sptr<Scene> scene);

  bool _renderNormals = false;

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

  static void setDrawType(DrawType drawType);

  static glm::mat4 getProjectMatrix();

  static unsigned int getPickingTextureID();

  static void renderDebugTexture(unsigned int textureID);
  static void renderDebugTexture(Texture *texture);

  static void setRenderNormals(bool value);
  static bool isRenderingNormals();

  static sptr<Shader> shader();

  static void renderScene(sptr<Scene> scene, sptr<Framebuffer> framebuffer);
  static void renderObject(sptr<Object> object, sptr<Scene> scene,
                           sptr<Framebuffer> framebuffer);

  static unsigned int pickObjAt(uint x, uint y, uint window_height);

  static void framebufferSizeCallback(int width, int height);

  static void pickingResize(float width, float height);

  static sptr<Shader> getGridShader() { return instance()._gridShader; }

  static sptr<VertexArray> getCubeVAO() { return instance().cubeVAO; }
  static sptr<VertexArray> getGridVAO() { return instance().gridVAO; }
  static sptr<VertexArray> getCircleVAO() { return instance().circleVAO; }
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
