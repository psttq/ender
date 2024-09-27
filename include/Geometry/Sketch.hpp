#pragma once
#include "Object.hpp"
#include "Topology/Wire.hpp"
#include <Spline1.hpp>
#include <string>

namespace EGEOM {
class Sketch : public ENDER::Object {
  sptr<Wire> _wire;

  Sketch(const std::string &name, sptr<Wire> wire);

public:
  std::string name;

  static sptr<Sketch> create(const std::string &name, sptr<Wire> wire);

  sptr<Wire> getWire();

  void setWire(sptr<Wire> wire);
};
} // namespace EGEOM
