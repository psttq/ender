#pragma once

#include "BufferObject.hpp"
#include <vector>
namespace ENDER {

class ArrayObject {
  unsigned int m_id;
  std::vector<unsigned int> m_vbos;

  unsigned int m_index = 0;

public:
  ArrayObject();

  ~ArrayObject() {
    if(m_id > 0) {
      glDeleteVertexArrays(1, &m_id);
      spdlog::info("Deallocated VAO. Index: {}.", m_id);
    }
  }

  void bind() const;

  void unbind() const;

  void addVBO(BufferObject *vbo);

};
} // namespace ENDER
