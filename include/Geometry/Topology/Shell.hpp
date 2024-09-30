#pragma once

#include "Object.hpp"
#include "Topology/Face.hpp"
namespace EGEOM {
class Shell : public ENDER::Object {
  std::vector<sptr<Face>> _faces;
  Shell();

public:
  bool shouldDrawGizmo = true;

public:
  static sptr<Shell> create();

  void addFace(sptr<Face> face);
  std::vector<sptr<Face>> getFaces();

  void drawGizmo() override;
};
} // namespace EGEOM
