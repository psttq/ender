#include <Sketch.hpp>

EGEOM::Sketch::Sketch(const std::string &name, sptr<EGEOM::Spline1> spline) : name(name), _spline(std::move(spline)){
}

sptr<EGEOM::Spline1> EGEOM::Sketch::getSpline() {
    return _spline;
}

void EGEOM::Sketch::setSpline(sptr<EGEOM::Spline1> spline) {
    _spline = spline;
}

sptr<EGEOM::Sketch> EGEOM::Sketch::create(const std::string &name, sptr<EGEOM::Spline1> spline) {
    return sptr<EGEOM::Sketch>(new Sketch(name, std::move(spline)));
}

sptr<ENDER::VertexArray> EGEOM::Sketch::getVAO() {
    return _spline->getVertexArray();
}
