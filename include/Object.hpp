#pragma once

#include "Texture.hpp"
#include "VertexArray.hpp"

namespace ENDER {
class Object {
  std::string m_name;
  VertexArray *m_vertexArray;
  Texture *m_texture;
public: 
  Object(const std::string &name, VertexArray *vertexArray);

  void setTexture(Texture *texture);
  Texture *getTexture() const;

  VertexArray *getVertexArray() const;

  std::string getName() const;
};
} // namespace ENDER
