#include <../../include/Renderer/VertexBuffer.hpp>
#include <../../3rd/spdlog/include/spdlog/spdlog.h>
#include <../../3rd/glad/include/glad/glad.h>

ENDER::VertexBuffer::VertexBuffer(uptr<BufferLayout> layout)
    : _layout(std::move(layout))
{
  glGenBuffers(1, &_id);
  spdlog::info("Created VBO. Index: {}", _id);
}

void ENDER::VertexBuffer::setData(float *data, unsigned int size)
{
  _count = size/sizeof(float);
  spdlog::info("Setting data to VBO. Index: {}. Size of data: {}", _id, size);
  bind();
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  // unbind();
}

void ENDER::VertexBuffer::bind()
{
  spdlog::debug("Bind VBO. Index: {}", _id);
  glBindBuffer(GL_ARRAY_BUFFER, _id);
}

void ENDER::VertexBuffer::unbind()
{
  spdlog::debug("Unbind VBO. Index: {}", _id);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int ENDER::VertexBuffer::getIndex() const { return _id; }

ENDER::BufferLayout &ENDER::VertexBuffer::getLayout() const
{
  return *_layout;
}
