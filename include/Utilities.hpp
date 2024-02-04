#include <Ender.hpp>
#include <ender_types.hpp>

namespace ENDER {
namespace Utils {

typedef std::function<glm::vec3(float, float)> ParametricSurfFunc;

inline unsigned int *generateParametricSurfaceGrid(int rows, int cols) {
  auto *indices = new unsigned int[(rows - 1) * (cols - 1) * 2 * 3];
  for (int row = 0; row < rows - 1; row++) {
    for (int col = 0; col < cols - 1; col++) {
      unsigned int f = col + row * cols;
      unsigned int s1 = col + (row + 1) * cols;
      unsigned int s2 = s1 + 1;
      unsigned int t = f + 1;
      indices[(col + row * (cols - 1)) * 6] = f;
      indices[(col + row * (cols - 1)) * 6 + 1] = s1;
      indices[(col + row * (cols - 1)) * 6 + 2] = s2;
      indices[(col + row * (cols - 1)) * 6 + 3] = f;
      indices[(col + row * (cols - 1)) * 6 + 4] = s2;
      indices[(col + row * (cols - 1)) * 6 + 5] = t;
    }
  }
  return indices;
}

inline Object *createParametricSurface(ParametricSurfFunc func, float u_max,
                                       float v_max, uint rows, uint cols) {
  auto indices = generateParametricSurfaceGrid(rows, cols);
  float *vertices = new float[rows * cols * 3];

  float h_u = u_max / (cols - 1);
  float h_v = v_max / (rows - 1);

  for (auto i = 0; i < rows; i++) {
    for (auto j = 0; j < cols; j++) {
      float u = h_u * j;
      float v = h_v * i;
      auto vertice = func(u, v);
      vertices[(j + i * cols) * 3] = vertice.x;     // x
      vertices[(j + i * cols) * 3 + 1] = vertice.y; // x
      vertices[(j + i * cols) * 3 + 2] = vertice.z; // x
    }
  }

  auto layout = new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}});
  auto vbo = new ENDER::VertexBuffer(layout);
  vbo->setData(vertices, sizeof(float) * rows * cols * 3);

  auto ibo = new ENDER::IndexBuffer(indices, (rows - 1) * (cols - 1) * 2 * 3);

  auto vao = new ENDER::VertexArray();
  vao->addVBO(vbo);
  vao->setIndexBuffer(ibo);

  auto object = new ENDER::Object("ParametricSurface", vao);

  return object;
}
} // namespace Utils
} // namespace ENDER
