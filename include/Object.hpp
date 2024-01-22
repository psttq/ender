#pragma once

#include "Texture.hpp"
#include "VertexArray.hpp"
#include <glm/glm.hpp>

namespace ENDER
{
  class Object
  {
    std::string _name;
    VertexArray *_vertexArray;
    Texture *_texture = nullptr;
    glm::vec3 _position{};
    glm::vec3 _rotation{};

  public:
    Object(const std::string &name, VertexArray *vertexArray);

    void setTexture(Texture *texture);
    Texture *getTexture() const;

    void setPosition(const glm::vec3 &position);
    void setRotation(const glm::vec3 &rotation);

    glm::vec3 getPosition() const;
    glm::vec3 getRotation() const;

    VertexArray *getVertexArray() const;

    std::string getName() const;
  };
} // namespace ENDER
