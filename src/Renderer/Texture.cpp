#include "../../3rd/spdlog/include/spdlog/spdlog.h"
#include <../../include/Renderer/Texture.hpp>
#include <../../3rd/glad/include/glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <../../3rd/stb/stb_image.h>

ENDER::Texture::Texture()
{
  glGenTextures(1, &_id);
  glBindTexture(GL_TEXTURE_2D, _id);
  // set the texture wrapping parameters
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

ENDER::Texture::~Texture() {
  spdlog::debug("Deallocation Texture[id: {}]", _id);
}

void ENDER::Texture::loadFromFile(const std::string &texturePath,
                                  unsigned int type)
{
  glBindTexture(GL_TEXTURE_2D, _id);
  stbi_set_flip_vertically_on_load(
      true);
  unsigned char *data =
      stbi_load(texturePath.c_str(), &_width, &_height, &_nrChannels, 0);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, type, _width, _height, 0, type,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    spdlog::info("Loaded texture[index: {}, path: {}]", _id, texturePath);
  }
  else
  {
    spdlog::error("Failed to load texture[path: {}]", texturePath);
  }
  stbi_image_free(data);
}

void ENDER::Texture::setAsCurrent()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _id);
}

unsigned int ENDER::Texture::getIndex() const { return _id; }
