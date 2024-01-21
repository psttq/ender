#include <IndexBuffer.hpp>
#include <glad/glad.h>

ENDER::IndexBuffer::IndexBuffer(unsigned int *indices, unsigned int count)
{
  _count = count;
  glGenBuffers(1, &_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices,
               GL_STATIC_DRAW);
  spdlog::debug("Created IndexBuffer. Index: {}", _id);
}

ENDER::IndexBuffer::~IndexBuffer()
{
  glDeleteBuffers(1, &_id);
  spdlog::info("Deallocated IndexBuffer[Index: {}]", _id);
}

void ENDER::IndexBuffer::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id); }

void ENDER::IndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
