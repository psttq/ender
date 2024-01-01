#include "ArrayObject.hpp"
#include "BufferObject.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"

ENDER::ArrayObject::ArrayObject() {
  glGenVertexArrays(1, &m_id);
  spdlog::info("VAO created. Index: {}", m_id);
}

void ENDER::ArrayObject::bind() {
  spdlog::debug("Bind VAO. Index: {}", m_id);
  glBindVertexArray(m_id);
}

void ENDER::ArrayObject::unbind() { 
  spdlog::debug("Unbind VAO. Index {}", m_id);
  glBindVertexArray(0); }

void ENDER::ArrayObject::addVBO(ENDER::BufferObject *vbo) {
  bind();
  vbo->bind();
  spdlog::info("Adding VBO[Index: {}] to VAO[Index: {}]", vbo->getIndex(),
               m_id);
  for (auto &el : vbo->getLayout()) {
    spdlog::debug("Applying vertex attribue with type {}:{}:{} ",
                  BufferLayout::convertTypeToString(el.type),
                  BufferLayout::convertTypeToGLType(el.type),
                  (const void *)el.offset);
    glVertexAttribPointer(m_index,
                          BufferLayout::convertTypeToNumberOfElements(el.type),
                          BufferLayout::convertTypeToGLType(el.type), GL_FALSE,
                          el.stride, (const void *)el.offset);
    glEnableVertexAttribArray(m_index);
    m_index++;
  }
  m_vbos.push_back(vbo->getIndex());
}
