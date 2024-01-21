#pragma once
#include <Object.hpp>
#include <Shader.hpp>

namespace ENDER
{
    // TODO: Singleton
    class Renderer
    {
        Shader *_shader;

    public:
        Renderer();
        ~Renderer();
        void renderObject(Object *object);
    };
} // namespace EMDER
