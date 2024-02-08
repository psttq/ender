#pragma once

#include <string>
namespace ENDER
{

  class Texture
  {
    unsigned int _id;
    int _width;
    int _height;
    int _nrChannels;

  public:
    Texture();
    ~Texture();

    void loadFromFile(const std::string &texturePath, unsigned int type);
    unsigned int getIndex() const;
    void setAsCurrent();
  };

} // namespace ENDER
