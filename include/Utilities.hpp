#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/Point_3.h>
#include <CGAL/Plane_3.h>

#include <Ender.hpp>
#include <ender_types.hpp>
namespace ENDER {
namespace Utils {



// CGAL типы
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Plane_3 Plane_3;
typedef K::Point_3 Point_3;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;

// Point3D convert_to_cgal_point(const glm::vec3 &glm_point);

sptr<VertexArray> createTriangulationSurfaceVAO(const std::vector<glm::vec3>
&curve_points, const std::vector<glm::vec3> &surface_points);

typedef std::function<glm::vec3(float, float)> ParametricSurfFunc;

unsigned int *generateParametricSurfaceGrid(int rows, int cols);

sptr<VertexArray> createParametricSurfaceVAO(ParametricSurfFunc func,
                                             float u_min, float v_min,
                                             float u_max, float v_max,
                                             uint rows, uint cols);

sptr<Object> createParametricSurface(ParametricSurfFunc func, float u_min,
                                     float v_min, float u_max, float v_max,
                                     uint rows, uint cols);
void applyImguiWhiteTheme();

void applyImguiTheme();
} // namespace Utils
} // namespace ENDER
