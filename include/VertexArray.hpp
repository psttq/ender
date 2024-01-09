#pragma once

#include "VertexBuffer.hpp"
#include <IndexBuffer.hpp>
#include <spdlog/spdlog.h>
#include <vector>
namespace ENDER {

class VertexArray {
  unsigned int m_id;
  std::vector<VertexBuffer *> m_vbos;
  IndexBuffer *m_indexBuffer;

  unsigned int m_index = 0;

public:
  VertexArray();

  ~VertexArray() {
    if(m_id > 0) {
      glDeleteVertexArrays(1, &m_id);
      spdlog::info("Deallocated VAO. Index: {}.", m_id);
    }
    for(auto vbo: m_vbos){
      delete vbo;
    }
  }

  void bind() const;

  void unbind() const;

  void setIndexBuffer(IndexBuffer *indexBuffer);

  void addVBO(VertexBuffer *vbo);

  unsigned int getIndex() const{
    return m_id;
  }

};
} // namespace ENDER
