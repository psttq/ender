#include <Topology/Shell.hpp>

namespace EGEOM {
Shell::Shell() : ENDER::Object("Shell") { type = ObjectType::Empty; }

sptr<Shell> Shell::create() { return sptr<Shell>(new Shell()); }

void Shell::addFace(sptr<Face> face) {
  _faces.push_back(face);
  addChildObject(face);
}

std::vector<sptr<Face>> Shell::getFaces() { return _faces; }

void Shell::drawGizmo() {
  if (shouldDrawGizmo)
    for (auto &face : _faces) {
      if (face->getSurface()->hovered())
        face->drawGizmo();
    }
  // _faces[_faces.size()-1]->drawGizmo();
}
} // namespace EGEOM
