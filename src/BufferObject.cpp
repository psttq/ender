#include "BufferObject.hpp"
#include "spdlog/spdlog.h"
#include <glad/glad.h>

ENDER::BufferObject::BufferObject(ENDER::BufferLayout *layout)
    : m_layout(layout) {
  glGenBuffers(1, &m_id);
  spdlog::info("Created VBO. Index: {}", m_id);
}

void ENDER::BufferObject::setData(float *data, unsigned int size) {
  spdlog::info("Setting data to VBO. Index: {}. Size of data: {}", m_id, size);
  bind();
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  // unbind();
}

void ENDER::BufferObject::bind() {
  spdlog::debug("Bind VBO. Index: {}", m_id);
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void ENDER::BufferObject::unbind() {
  spdlog::debug("Unbind VBO. Index: {}", m_id);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int ENDER::BufferObject::getIndex() const { return m_id; }

ENDER::BufferLayout &ENDER::BufferObject::getLayout() const {
  return *m_layout;
}
