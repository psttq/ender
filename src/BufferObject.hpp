#pragma once

#include "BufferLayout.hpp"
namespace ENDER {
class BufferObject {
  unsigned int m_id = 0;
  BufferLayout *m_layout;

public:
  BufferObject(BufferLayout *layout);
  ~BufferObject(){
    delete m_layout;
  }

  void bind();
  void unbind();

  unsigned int getIndex() const;

  BufferLayout &getLayout() const;

  void setData(float *data);
};
} // namespace ENDER
