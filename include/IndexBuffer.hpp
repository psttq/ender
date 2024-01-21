#pragma once

#include <spdlog/spdlog.h>

namespace ENDER
{

  class IndexBuffer
  {
    unsigned int _id;
    unsigned int _count;

  public:
    IndexBuffer(unsigned int *indices, unsigned int count);
    ~IndexBuffer();

    void bind();
    void unbind();
    unsigned int getIndex()
    {
      return _id;
    }
  };
}; // namespace ENDER
