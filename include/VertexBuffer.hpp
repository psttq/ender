#pragma once

#include "BufferLayout.hpp"
namespace ENDER
{
  class VertexBuffer
  {
    unsigned int _id = 0;
    BufferLayout *_layout;

  public:
    VertexBuffer(BufferLayout *layout);
    ~VertexBuffer()
    {
      if (_id > 0)
      {
        glDeleteBuffers(1, &_id);
        spdlog::info("Deallocated VBO. Index: {}.", _id);
      }
      delete _layout;
    }

    void bind();
    void unbind();

    unsigned int getIndex() const;

    BufferLayout &getLayout() const;

    void setData(float *data, unsigned int size);
  };
} // namespace ENDER
