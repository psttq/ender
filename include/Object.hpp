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
    glm::vec3 _position;

  public:
    Object(const std::string &name, VertexArray *vertexArray);

    void setTexture(Texture *texture);
    Texture *getTexture() const;

    void setPosition(const glm::vec3 &position);

    glm::vec3 getPosition() const;

    VertexArray *getVertexArray() const;

    std::string getName() const;
  };
} // namespace ENDER
