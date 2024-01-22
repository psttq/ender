#pragma once
#include <Object.hpp>
#include <Shader.hpp>
#include <Window.hpp>

namespace ENDER
{
    // TODO: Singleton
    class Renderer
    {
        Shader *_shader;
        glm::mat4 _projectMatrix;
        glm::mat4 _viewMatrix;

        Renderer();
        ~Renderer();

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
        static void renderObject(Object *object);

        static void swapBuffers();

        static void framebufferSizeCallback(int width, int height);
    };
} // namespace EMDER
