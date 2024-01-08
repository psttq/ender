#include "spdlog/spdlog.h"
#include <Texture.hpp>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

ENDER::Texture::Texture() {
  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_2D, m_id);
  // set the texture wrapping parameters
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ENDER::Texture::loadFromFile(const std::string &texturePath,
                                  unsigned int type) {
  glBindTexture(GL_TEXTURE_2D, m_id);
  stbi_set_flip_vertically_on_load(
      true); // tell stb_image.h to flip loaded texture's on the y-axis.
  // The FileSystem::getPath(...) is part of the GitHub repository so we can
  // find files on any IDE/platform; replace it with your own image path.
  unsigned char *data =
      stbi_load(texturePath.c_str(), &m_width, &m_height, &m_nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, type, m_width, m_height, 0, type,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    spdlog::info("Loaded texture[index: {}, path: {}]", m_id, texturePath);
  } else {
    spdlog::error("Failed to load texture[path: {}]", texturePath);
  }
  stbi_image_free(data);
}

unsigned int ENDER::Texture::getIndex() const { return m_id; }
