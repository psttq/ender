#include <PivotPlane.hpp>

sptr<ENDER::VertexArray> EGEOM::PivotPlane::planeVAO = nullptr;
sptr<ENDER::Shader> EGEOM::PivotPlane::planeShader = nullptr;

EGEOM::PivotPlane::PivotPlane(const std::string &name, sptr<ENDER::VertexArray> vertexArray) : Object(name,
                                                                                                      vertexArray) {
    setShader(planeShader);

}

sptr<EGEOM::PivotPlane> EGEOM::PivotPlane::create(const std::string &name) {
    if (!planeVAO) {
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
        planeShader = ENDER::Shader::create("../resources/shaders/planeShader.vs",
                                            "../resources/shaders/planeShader.fs");
    }
    return sptr<PivotPlane>(new PivotPlane(name, planeVAO));
}

void EGEOM::PivotPlane::setSketch(sptr<EGEOM::Sketch> sketch) {
    _currentSketch = sketch;
    auto newObj = ENDER::Object::create("PivotPlane_Child", sketch->getVAO());
    newObj->type = ObjectType::Line;
    setChildObject(newObj);
}

void EGEOM::PivotPlane::drawProperties(const std::vector<sptr<Sketch>> &sketches) {
    Object::drawProperties();

    int currentItem = 0;

    std::vector<const char *> items;

    items.push_back("None");
    int i = 1;
    std::for_each(sketches.begin(), sketches.end(),
                  [&](const sptr<Sketch> &sketch) {
                      items.push_back(sketch->name.c_str());
                      if (sketch == _currentSketch)
                          currentItem = i;
                      i++;
                  });

    if (ImGui::TreeNode("Pivot Plane")) {
        if (ImGui::Combo("Sketches", &currentItem, &items[0], items.size())) {
            if(currentItem == 0){
                resetChildObject();
                _currentSketch = nullptr;
            }
            else
                setSketch(sketches[currentItem-1]);
        }
        ImGui::TreePop();
    }
}
