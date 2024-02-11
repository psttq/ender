#pragma once
#include "PickingTexture.hpp"
#include <ender_types.hpp>

namespace ENDER {
    class Framebuffer {
        uint _id;
        uint _rid;
        uint _tid;

        float _width;
        float _height;

        sptr<PickingTexture> _pickingTexture;

        Framebuffer(float width, float height);

    public:
        ~Framebuffer();

        static sptr<Framebuffer> create(float width, float height);

        uint getId();

        uint getTextureId();

        sptr<PickingTexture> getPickingTexture();

        void clear();

        uint pickObjAt(uint x, uint y); 
        // here we bind our framebuffer
        void bind();

        // here we unbind our framebuffer
        void unbind();

        // and we rescale the buffer, so we're able to resize the window
        void rescale(float width, float height);
    };
}
