#include <IndexBuffer.hpp>
#include <glad/glad.h>

ENDER::IndexBuffer::IndexBuffer(unsigned int *indices, unsigned int count) {
  m_count = count;
  glGenBuffers(1, &m_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices,
               GL_STATIC_DRAW);
  spdlog::debug("Created IndexBuffer. Index: {}", m_id);
}

ENDER::IndexBuffer::~IndexBuffer() { glDeleteBuffers(1, &m_id); spdlog::info("Deallocated IndexBuffer[Index: {}]", m_id); }

void ENDER::IndexBuffer::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id); }

void ENDER::IndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
