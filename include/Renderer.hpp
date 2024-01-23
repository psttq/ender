#pragma once
#include <Object.hpp>
#include <Scene.hpp>
#include <Shader.hpp>
#include <Window.hpp>

namespace ENDER
{
    // TODO: Singleton
    class Renderer
    {
        Shader *_shader;
        glm::mat4 _projectMatrix;

        VertexArray *cubeVAO;

        Renderer();
        ~Renderer();

        void createCubeVAO();

        void renderObject(Object *object);

    public:
        static Renderer &instance()
        {
            static Renderer _instance;
            return _instance;
        }

        static void init();

        static void setClearColor(const glm::vec4 &color);
        // static void swapBuffers(const glm::vec4 &color);

        static void clear();

        static void swapBuffers();

        static void renderScene(Scene *scene);

        static void framebufferSizeCallback(int width, int height);

        static VertexArray *getCubeVAO()
        {
            return instance().cubeVAO;
        }
    };

} // namespace EMDER
