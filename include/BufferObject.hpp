#pragma once

#include "BufferLayout.hpp"
namespace ENDER {
class BufferObject {
  unsigned int m_id = 0;
  BufferLayout *m_layout;

public:
  BufferObject(BufferLayout *layout);
  ~BufferObject(){
    if(m_id > 0) {
      glDeleteBuffers(1, &m_id);
      spdlog::info("Deallocated VBO. Index: {}.", m_id);
    }
    delete m_layout;

  }

  void bind();
  void unbind();

  unsigned int getIndex() const;

  BufferLayout &getLayout() const;

  void setData(float *data, unsigned int size);
};
} // namespace ENDER
