#pragma once

#include "BufferObject.hpp"
#include <vector>
namespace ENDER {

class ArrayObject {
  unsigned int m_id;
  std::vector<int> m_vbos;

  unsigned int m_index = 0;

public:
  ArrayObject();

  void bind();

  void unbind();

  void addVBO(BufferObject *vbo);

};
} // namespace ENDER
