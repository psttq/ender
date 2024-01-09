#include "VertexArray.hpp"
#include <Object.hpp>

ENDER::Object::Object(const std::string &name, VertexArray *vertexArray) : m_name(name), m_vertexArray(vertexArray){
  spdlog::debug("Created object[name: {}] with VertexArray[index: {}]", name, vertexArray->getIndex());
}

void ENDER::Object::setTexture(Texture *texture){
  m_texture = texture;
}

ENDER::Texture *ENDER::Object::getTexture() const{
  return m_texture;
}

std::string ENDER::Object::getName() const{
  return m_name;
}

ENDER::VertexArray *ENDER::Object::getVertexArray() const{
  return m_vertexArray;
}



