#include "../../include/Renderer/VertexArray.hpp"
#include "imgui.h"
#include <../../3rd/glm/glm/glm.hpp>
#include <../../include/Renderer/Object.hpp>
#include <../../include/Renderer/Renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <memory>

bool ENDER::Object::selected() const { return _selected; }

void ENDER::Object::setSelected(bool selected) { this->_selected = selected; }



ENDER::Object::Object(const std::string &name, sptr<VertexArray> vertexArray)
    : _name(name), _vertexArray(vertexArray) {
  static unsigned int _objCount = 1;
  _id = _objCount;
  _objCount++;
  spdlog::debug("Created object[name: {}, id: {}] with VertexArray[index: {}]",
                name, _id, vertexArray->getIndex());
}

ENDER::Object::Object(const std::string &name) : _name(name) {
  static unsigned int _objCount = 1;
  _id = _objCount;
  _objCount++;
  spdlog::debug("Created object[name: {}, id: {}] without VertexArray", name,
                _id);
}

unsigned int ENDER::Object::getId() const { return _id; }

void ENDER::Object::setTexture(Texture *texture) { _texture = texture; }

void ENDER::Object::setPosition(const glm::vec3 &position) {
  _position = position;
}

glm::vec3 &ENDER::Object::getPosition() { return _position; }

void ENDER::Object::setRotation(const glm::vec3 &rotation) {
  _rotation = rotation;
}

void ENDER::Object::setScale(const glm::vec3 &scale) { _scale = scale; }

glm::vec3 &ENDER::Object::getRotation() { return _rotation; }

glm::vec3 &ENDER::Object::getScale() { return _scale; }

const std::string &ENDER::Object::getName() { return _name; }

void ENDER::Object::setShader(sptr<Shader> shader) { _shader = shader; }

sptr<ENDER::Shader> ENDER::Object::getShader() { return _shader; }

ENDER::Texture *ENDER::Object::getTexture() const { return _texture; }

std::string ENDER::Object::getName() const { return _name; }

sptr<ENDER::VertexArray> ENDER::Object::getVertexArray() const {
  return _vertexArray;
}

void ENDER::Object::setVertexArray(sptr<VertexArray> vertexArray) {
  _vertexArray = vertexArray;
}

sptr<ENDER::Object> ENDER::Object::create(const std::string &name,
                                          sptr<VertexArray> vertexArray) {
  return std::make_shared<Object>(name, vertexArray);
}

sptr<ENDER::Object> ENDER::Object::createCube(const std::string &name) {
  return create(name, Renderer::getCubeVAO());
}

sptr<ENDER::Object> ENDER::Object::createGrid(const std::string &name) {
  auto grid = create(name, Renderer::getGridVAO());
  grid->setShader(Renderer::getGridShader());
  return grid;
}

void ENDER::Object::addChildObject(sptr<ENDER::Object> childObject) {
  _children.push_back(childObject);
}

void ENDER::Object::deleteChildObject(sptr<Object> childObject){
  _children.erase(std::remove(_children.begin(), _children.end(), childObject), _children.end());
}


void ENDER::Object::deleteAllChildren() {
  _children.clear();
  type = ObjectType::Surface;
}

std::vector<sptr<ENDER::Object>> ENDER::Object::getChildren() {
  return _children;
}

bool ENDER::Object::hasChildren() const { return !_children.empty(); }

void ENDER::Object::drawProperties() {
  if (ImGui::TreeNode("Transform")) {
    ImGui::InputFloat3("Position", glm::value_ptr(_position));
    ImGui::InputFloat3("Rotation", glm::value_ptr(_rotation));
    ImGui::InputFloat3("Scale", glm::value_ptr(_scale));
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Material")) {
    material.drawImguiEdit();
    ImGui::TreePop();
  }
}

glm::mat4 ENDER::Object::getTransform() const {
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, _position);

  glm::vec3 xNorm(1.0, 0.0f, 0.0);
  glm::vec3 yNorm(0.0, 1.0f, 0.0);
  glm::vec3 zNorm(0.0, 0.0f, 1.0);

  model = glm::rotate(model, _rotation.x, xNorm); // Rotate on X axis
  yNorm = glm::rotate(yNorm, -_rotation.x, xNorm);
  zNorm = glm::rotate(zNorm, -_rotation.x, xNorm);
  model = glm::rotate(model, _rotation.y, yNorm); // Rotate on Y axis
  zNorm = glm::rotate(zNorm, -_rotation.y, yNorm);
  model = glm::rotate(model, _rotation.z, zNorm); // Rotate

  model = glm::scale(model, _scale);
  return model;
}

glm::vec3 ENDER::Object::getPosition() const { return _position; }

glm::vec3 ENDER::Object::getRotation() const { return _rotation; }

glm::vec3 ENDER::Object::getScale() const { return _scale; }

void ENDER::Object::setName(const std::string &name) { _name = name; }