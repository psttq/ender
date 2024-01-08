#include <VertexBuffer.hpp>
#include <spdlog/spdlog.h>
#include <glad/glad.h>

ENDER::VertexBuffer::VertexBuffer(ENDER::BufferLayout *layout)
    : m_layout(layout) {
  glGenBuffers(1, &m_id);
  spdlog::info("Created VBO. Index: {}", m_id);
}

void ENDER::VertexBuffer::setData(float *data, unsigned int size) {
  spdlog::info("Setting data to VBO. Index: {}. Size of data: {}", m_id, size);
  bind();
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  // unbind();
}

void ENDER::VertexBuffer::bind() {
  spdlog::debug("Bind VBO. Index: {}", m_id);
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void ENDER::VertexBuffer::unbind() {
  spdlog::debug("Unbind VBO. Index: {}", m_id);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int ENDER::VertexBuffer::getIndex() const { return m_id; }

ENDER::BufferLayout &ENDER::VertexBuffer::getLayout() const {
  return *m_layout;
}
