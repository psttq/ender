#pragma once

#include "VertexBuffer.hpp"
#include <IndexBuffer.hpp>
#include <spdlog/spdlog.h>
#include <vector>
namespace ENDER
{

  class VertexArray
  {
    unsigned int _id;
    std::vector<uptr<VertexBuffer>> _vbos;
    uptr<IndexBuffer> _indexBuffer = nullptr;
    sptr<spdlog::logger> logger;

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
    }

    void bind() const;

    void unbind() const;

    void setIndexBuffer(uptr<IndexBuffer> indexBuffer);

    void setVBOdata(uint vboIndex, float *data, uint size);

    void addVBO(uptr<VertexBuffer> vbo);

    bool isIndexBuffer() const;

    unsigned int indexCount();
    uint verticesCount();

    unsigned int getIndex() const
    {
      return _id;
    }
  };
} // namespace ENDER
