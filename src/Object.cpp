#include "VertexArray.hpp"
#include <Object.hpp>
#include <glm/glm.hpp>

ENDER::Object::Object(const std::string &name, VertexArray *vertexArray) : _name(name), _vertexArray(vertexArray)
{
  spdlog::debug("Created object[name: {}] with VertexArray[index: {}]", name, vertexArray->getIndex());
}

void ENDER::Object::setTexture(Texture *texture)
{
  _texture = texture;
}

void ENDER::Object::setPosition(const glm::vec3 &position)
{
  _position = position;
}

glm::vec3 ENDER::Object::getPosition() const
{
  return _position;
}

void ENDER::Object::setRotation(const glm::vec3 &rotation)
{
  _rotation = rotation;
}

glm::vec3 ENDER::Object::getRotation() const
{
  return _rotation;
}

ENDER::Texture *ENDER::Object::getTexture() const
{
  return _texture;
}

std::string ENDER::Object::getName() const
{
  return _name;
}

ENDER::VertexArray *ENDER::Object::getVertexArray() const
{
  return _vertexArray;
}
