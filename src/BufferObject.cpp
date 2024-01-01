#include "BufferObject.hpp"
#include <glad/glad.h>

ENDER::BufferObject::BufferObject(ENDER::BufferLayout *layout)
    : m_layout(layout) {
  glGenBuffers(1, &m_id);
  spdlog::info("Created VBO. Index: {}", m_id);
}

void ENDER::BufferObject::setData(float *data) {
  spdlog::info("Setting data to VBO. Index: {}", m_id);
  bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  // unbind();
}

void ENDER::BufferObject::bind() { glBindBuffer(GL_ARRAY_BUFFER, m_id); }

void ENDER::BufferObject::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

unsigned int ENDER::BufferObject::getIndex() const { return m_id; }

ENDER::BufferLayout &ENDER::BufferObject::getLayout() const {
  return *m_layout;
}
