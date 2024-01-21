#include <VertexArray.hpp>
#include <VertexBuffer.hpp>
#include <glad/glad.h>

ENDER::VertexArray::VertexArray()
{
  glGenVertexArrays(1, &_id);
  spdlog::info("VAO created. Index: {}", _id);
}

void ENDER::VertexArray::bind() const
{
  // spdlog::debug("Bind VAO. Index: {}", m_id);
  glBindVertexArray(_id);
}

void ENDER::VertexArray::unbind() const
{
  // spdlog::debug("Unbind VAO. Index {}", m_id);
  glBindVertexArray(0);
}

void ENDER::VertexArray::addVBO(ENDER::VertexBuffer *vbo)
{
  bind();
  vbo->bind();
  spdlog::info("Adding VBO[Index: {}] to VAO[Index: {}]", vbo->getIndex(),
               _id);
  for (auto &el : vbo->getLayout())
  {
    spdlog::debug("Applying vertex attribue with type {}",
                  BufferLayout::convertTypeToString(el.type));
    glVertexAttribPointer(_index,
                          BufferLayout::convertTypeToNumberOfElements(el.type),
                          BufferLayout::convertTypeToGLType(el.type), GL_FALSE,
                          el.stride, (const void *)el.offset);
    glEnableVertexAttribArray(_index);
    _index++;
  }
  _vbos.push_back(vbo);
}

void ENDER::VertexArray::setIndexBuffer(ENDER::IndexBuffer *indexBuffer)
{
  bind();
  indexBuffer->bind();
  _indexBuffer = indexBuffer;
  spdlog::info("Adding IndexBuffer[Index: {}] to VAO[Index: {}]", indexBuffer->getIndex(), _id);
}
