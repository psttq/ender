#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_3.h>
#include <Ender.hpp>
#include <ender_types.hpp>
namespace ENDER {
namespace Utils {

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point_3;
typedef CGAL::Delaunay_triangulation_3<K> Triangulation;

Point_3 convert_to_cgal_point(const glm::vec3 &glm_point);

Triangulation triangulatePoints(const std::vector<glm::vec3> &points);
sptr<VertexArray>
createTriangulationSurfaceVAO(const std::vector<glm::vec3> &points);

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
