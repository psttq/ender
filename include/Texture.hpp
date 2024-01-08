#pragma once

#include <string>
namespace ENDER {

class Texture {
  unsigned int m_id;
  int m_width;
  int m_height;
  int m_nrChannels;

public:
  Texture();
  ~Texture();

  void loadFromFile(const std::string &texturePath, unsigned int type);
  unsigned int getIndex() const;

};

} // namespace ENDER
