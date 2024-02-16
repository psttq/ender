#include <../../include/Renderer/VertexArray.hpp>
#include <../../include/Renderer/VertexBuffer.hpp>
#include <../../3rd/glad/include/glad/glad.h>

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

void ENDER::VertexArray::addVBO(uptr<VertexBuffer> vbo)
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
  _vbos.push_back(std::move(vbo));
}

bool ENDER::VertexArray::isIndexBuffer() const {
  return _indexBuffer != nullptr;
}

uint ENDER::VertexArray::verticesCount(){
  uint res = 0;
  for(auto &vbo : _vbos){
    res += vbo->count();
  }
  return res;
}

unsigned int ENDER::VertexArray::indexCount() {
  if(!isIndexBuffer()) {
    spdlog::error("Trying to get index buffer elements count but there is no index buffer in the vertex array[id: {}]", _id);
    return 0;
  }
  return _indexBuffer->getCount();
}

void ENDER::VertexArray::setIndexBuffer(uptr<IndexBuffer> indexBuffer)
{
  bind();
  indexBuffer->bind();

  _indexBuffer = std::move(indexBuffer);
  spdlog::info("Adding IndexBuffer[Index: {}] to VAO[Index: {}]", _indexBuffer->getIndex(), _id);

}
