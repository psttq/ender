#pragma once

#include "BufferLayout.hpp"
#include <ender_types.hpp>

namespace ENDER
{
  class VertexBuffer
  {
    unsigned int _id = 0;
    uptr<BufferLayout> _layout;
    uint _count = 0;

  public:
    VertexBuffer(uptr<BufferLayout> layout);
    ~VertexBuffer()
    {
      if (_id > 0)
      {
        glDeleteBuffers(1, &_id);
        spdlog::info("Deallocated VBO. Index: {}.", _id);
      };
    }

    void bind();
    void unbind();

    unsigned int getIndex() const;

    BufferLayout &getLayout() const;

    void setData(float *data, unsigned int size);

    uint count() const { return _count; }
  };
} // namespace ENDER
