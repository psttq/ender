#pragma once
#include <Object.hpp>
#include <Sketch.hpp>

namespace EGEOM {
    class PivotPlane : public ENDER::Object {

        PivotPlane(const std::string &name, sptr<ENDER::VertexArray> vertexArray);
    public:
        static sptr<PivotPlane> create(const std::string &name);

        void setSketch(sptr<EGEOM::Sketch> sketch);

        static sptr<ENDER::VertexArray> planeVAO;
        static sptr<ENDER::Shader> planeShader;

        void drawProperties() override;
    };

    static float planeVertices[] = {
            // positions       // texture coords
            0.5f,  0.0f,0.5f,   1.0f, 1.0f, // top right
            0.5f,  0.0f,-0.5f,  1.0f, 0.0f, // bottom right
            -0.5f, 0.0f,-0.5f,  0.0f, 0.0f, // bottom left
            -0.5f, 0.0f,0.5f,   0.0f, 1.0f, // top left
            0.5f,  0.0f,0.5f,   1.0f, 1.0f, // top right
            -0.5f, 0.0f,-0.5f,  0.0f, 0.0f, // bottom left
    };
    static unsigned int planeIndices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };

}