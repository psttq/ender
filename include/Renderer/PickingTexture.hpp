#pragma once

#include <../../3rd/spdlog/include/spdlog/spdlog.h>
#include <../../3rd/glad/include/glad/glad.h>
#include <ender_types.hpp>

namespace ENDER {
    class PickingTexture {
    public:
        PickingTexture() {}

        ~PickingTexture();

        static sptr<PickingTexture> create();

        void init(unsigned int windowWidth, unsigned int windowHeight);

        void enableWriting();

        void disableWriting();

        void updateTextureSize(unsigned int width, unsigned int height);

        unsigned int getTextureID();

        struct PixelInfo {
            unsigned int objectID = 0;
            unsigned int drawID = 0;
            unsigned int primID = 0;

            void print() {
                spdlog::debug("Object %d draw %d prim %d\n", objectID, drawID, primID);
            }
        };

        PixelInfo readPixel(unsigned int x, unsigned int y);

    private:
        GLuint _fbo = 0;
        GLuint _pickingTexture = 0;
        GLuint _depthTexture = 0;
    };

}
