#pragma once
#include <ender_types.hpp>

namespace ENDER {
    class Framebuffer {
        uint _id;
        uint _rid;
        uint _tid;

        float _width;
        float _height;

        Framebuffer(float width, float height);



    public:
        ~Framebuffer();

        static sptr<Framebuffer> create(float width, float height);

        uint getId();

        uint getTextureId();

        // here we bind our framebuffer
        void bind();

        // here we unbind our framebuffer
        void unbind();

        // and we rescale the buffer, so we're able to resize the window
        void rescale(float width, float height);
    };
}
