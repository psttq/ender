#include <Spline1.hpp>

namespace EGEOM
{
    Spline1::Spline1(const std::vector<sptr<Point>> &points, ParamMethod paramMethod, uint interpolatedPointsCount) : ENDER::Object("Spline1")
    {
        type = ObjectType::Line;

        _points = points;
        _interpolatedPointsCount = interpolatedPointsCount;
        _paramMethod = paramMethod;

        auto layout = uptr<ENDER::BufferLayout>(
            new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));

        auto vbo = std::make_unique<ENDER::VertexBuffer>(std::move(layout));
        vbo->setData(&_rawData[0], _rawData.size() * sizeof(float));
        _vertexArray = std::make_shared<ENDER::VertexArray>();
        _vertexArray->addVBO(std::move(vbo));
        _calculateParameter();
        _calculateDrawPoints();
    }

    void Spline1::_calculateParameter()
    {
        switch (_paramMethod)
        {
        case ParamMethod::Uniform:
        {
            _calculateUniformParameter();
        }
        break;
        default:
        {
            spdlog::error("Spline1::_calculateParameter: Not implemented");
        }
        }
    }

    void Spline1::_calculateUniformParameter()
    {
        _t.clear();
        for (auto i = 0; i < _points.size(); i++)
        {
            _t.push_back(static_cast<float>(i) / (static_cast<float>(_points.size()) - 1.0f));
        }
    }

    void Spline1::_calculateDrawPoints()
    {
        if(_points.size() < 2)
            return;
        _interpolatedPoints.clear();
        for (auto i = 0; i < _interpolatedPointsCount; i++)
        {
            float t = i * 1.f / (_interpolatedPointsCount - 1);
            uint j = 0;
            while (t > _t[j+1])
                j++;

            float h = _t[j + 1] - _t[j];
            float omega = (t - _t[j]) / h;

            glm::vec3 pointPosition;
            pointPosition.x = _points[j]->getPosition().x * (1.0f - omega) + _points[j+1]->getPosition().x * omega;
            pointPosition.y = _points[j]->getPosition().y * (1.0f - omega) + _points[j+1]->getPosition().y * omega;
            pointPosition.z = _points[j]->getPosition().z * (1.0f - omega) + _points[j+1]->getPosition().z * omega;

            _interpolatedPoints.push_back(Point::create(pointPosition));
        }
        _rawData.clear();
        for (auto point : _interpolatedPoints)
        {
            _rawData.insert(_rawData.end(),
                            {point->getPosition().x, point->getPosition().y,
                             point->getPosition().z});
        }
        _vertexArray->setVBOdata(0, &_rawData[0], _rawData.size() * sizeof(float));
    }
    void Spline1::setPoints(const std::vector<sptr<Point>> &points)
    {
        _points = points;
        _calculateParameter();
        _calculateDrawPoints();
    }

    std::vector<sptr<Point>> Spline1::getInterpolatedPoints() {
        return _interpolatedPoints;
    }

    void Spline1::setInterpolationPointsCount(uint count) {
        _interpolatedPointsCount = count;
    }

    void Spline1::update() {
        _calculateParameter();
        _calculateDrawPoints();
    }

} // namespace EGEOM
