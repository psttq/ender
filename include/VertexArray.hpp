#pragma once

#include "VertexBuffer.hpp"
#include <IndexBuffer.hpp>
#include <spdlog/spdlog.h>
#include <vector>
namespace ENDER
{

  class VertexArray {
    unsigned int _id;
    std::vector<VertexBuffer *> _vbos;
    IndexBuffer *_indexBuffer = nullptr;

    unsigned int _index = 0;

  public:
    VertexArray();

    ~VertexArray()
    {
      if (_id > 0)
      {
        glDeleteVertexArrays(1, &_id);
        spdlog::info("Deallocated VAO. Index: {}.", _id);
      }
      for (auto vbo : _vbos)
      {
        delete vbo;
      }
    }

    void bind() const;

    void unbind() const;

    void setIndexBuffer(IndexBuffer *indexBuffer);

    void addVBO(VertexBuffer *vbo);

    bool isIndexBuffer() const;

    unsigned int indexCount();

    unsigned int getIndex() const
    {
      return _id;
    }
  };
} // namespace ENDER
