#include <VertexArray.hpp>
#include <VertexBuffer.hpp>
#include <glad/glad.h>


ENDER::VertexArray::VertexArray() {
  glGenVertexArrays(1, &m_id);
  spdlog::info("VAO created. Index: {}", m_id);
}

void ENDER::VertexArray::bind() const {
  // spdlog::debug("Bind VAO. Index: {}", m_id);
  glBindVertexArray(m_id);
}

void ENDER::VertexArray::unbind() const {
  // spdlog::debug("Unbind VAO. Index {}", m_id);
  glBindVertexArray(0);
}

void ENDER::VertexArray::addVBO(ENDER::VertexBuffer *vbo) {
  bind();
  vbo->bind();
  spdlog::info("Adding VBO[Index: {}] to VAO[Index: {}]", vbo->getIndex(),
               m_id);
  for (auto &el : vbo->getLayout()) {
    spdlog::debug("Applying vertex attribue with type {}",
                  BufferLayout::convertTypeToString(el.type));
    glVertexAttribPointer(m_index,
                          BufferLayout::convertTypeToNumberOfElements(el.type),
                          BufferLayout::convertTypeToGLType(el.type), GL_FALSE,
                          el.stride, (const void *)el.offset);
    glEnableVertexAttribArray(m_index);
    m_index++;
  }
  m_vbos.push_back(vbo);
}

void ENDER::VertexArray::setIndexBuffer(ENDER::IndexBuffer *indexBuffer){
  bind();
  indexBuffer->bind();
  m_indexBuffer = indexBuffer;
  spdlog::info("Adding IndexBuffer[Index: {}] to VAO[Index: {}]", indexBuffer->getIndex(), m_id);
}
