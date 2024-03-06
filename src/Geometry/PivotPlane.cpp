#include <PivotPlane.hpp>

sptr<ENDER::VertexArray> EGEOM::PivotPlane::planeVAO = nullptr;
sptr<ENDER::Shader> EGEOM::PivotPlane::planeShader = nullptr;

EGEOM::PivotPlane::PivotPlane(const std::string &name, sptr<ENDER::VertexArray> vertexArray) : Object(name, vertexArray) {
    setShader(planeShader);

}
sptr<EGEOM::PivotPlane> EGEOM::PivotPlane::create(const std::string &name) {
    if(!planeVAO) {
        auto planeLayout = uptr<ENDER::BufferLayout>(new ENDER::BufferLayout(
                {{ENDER::LayoutObjectType::Float3},
                 {ENDER::LayoutObjectType::Float2}}));
        auto planeVBO = std::make_unique<ENDER::VertexBuffer>(std::move(planeLayout));
        planeVBO->setData(planeVertices,
                          sizeof(planeVertices));
        auto planeIBO =
                std::make_unique<ENDER::IndexBuffer>(planeIndices, 6);
        planeVAO = std::make_shared<ENDER::VertexArray>();
        planeVAO->addVBO(std::move(planeVBO));
        planeVAO->setIndexBuffer(std::move(planeIBO));
        planeShader = ENDER::Shader::create("../resources/shaders/planeShader.vs", "../resources/shaders/planeShader.fs");
    }
    return sptr<PivotPlane>(new PivotPlane(name, planeVAO));
}

void EGEOM::PivotPlane::setSketch(sptr<EGEOM::Sketch> sketch) {
    auto newObj = ENDER::Object::create("PivotPlane_Child", sketch->getVAO());
    newObj->type = ObjectType::Line;
    setChildObject(newObj);
}
