#include <Sketch.hpp>

EGEOM::Sketch::Sketch(const std::string &name, sptr<EGEOM::Wire> wire)
    : ENDER::Object(name), name(name), _wire(std::move(wire)) {
  type = ObjectType::Empty;
   for (auto edge : _wire->getEdges()) {
    edge->setPosition(getPosition());
    addChildObject(edge);
  }
}

sptr<EGEOM::Wire> EGEOM::Sketch::getWire() { return _wire; }

void EGEOM::Sketch::setWire(sptr<EGEOM::Wire> wire) {
  _wire = wire;
  for (auto edge : _wire->getEdges()) {
    addChildObject(edge);
  }
}

sptr<EGEOM::Sketch> EGEOM::Sketch::create(const std::string &name,
                                          sptr<EGEOM::Wire> wire) {
  return sptr<EGEOM::Sketch>(new Sketch(name, std::move(wire)));
}
