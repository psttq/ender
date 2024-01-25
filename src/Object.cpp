#include "VertexArray.hpp"
#include <Object.hpp>
#include <glm/glm.hpp>
#include <Renderer.hpp>

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

void ENDER::Object::setScale(const glm::vec3 &scale) {
  _scale = scale;
}

glm::vec3 ENDER::Object::getRotation() const
{
  return _rotation;
}

glm::vec3 ENDER::Object::getScale() const {
  return _scale;
}

void ENDER::Object::setShader(Shader *shader) {
  _shader = shader;
}

ENDER::Shader * ENDER::Object::getShader() {
  return _shader;
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

ENDER::Object *ENDER::Object::createCube(const std::string &name)
{
  Object *cube = new Object(name, Renderer::getCubeVAO());
  return cube;
}
