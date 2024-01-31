#include "PickingTexture.hpp"

#include <glad/glad.h>

namespace ENDER {
PickingTexture::~PickingTexture() {
  if (_fbo != 0) {
    glDeleteFramebuffers(1, &_fbo);
  }

  if (_pickingTexture != 0) {
    glDeleteTextures(1, &_pickingTexture);
  }

  if (_depthTexture != 0) {
    glDeleteTextures(1, &_depthTexture);
  }
}

unsigned int PickingTexture::getTextureID() { return _pickingTexture; }

void PickingTexture::init(unsigned int windowWidth, unsigned int windowHeight) {
  // Create the FBO
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  spdlog::debug("Generated framebuffer: {}", _fbo);

  // Create the texture object for the primitive information buffer
  glGenTextures(1, &_pickingTexture);
  glBindTexture(GL_TEXTURE_2D, _pickingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, windowWidth, windowHeight, 0,
               GL_RGB_INTEGER, GL_UNSIGNED_INT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _pickingTexture, 0);

  // Create the texture object for the depth buffer
  glGenTextures(1, &_depthTexture);
  glBindTexture(GL_TEXTURE_2D, _depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight,
               0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         _depthTexture, 0);

  // Verify that the FBO is correct
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    spdlog::error("FB error, status: 0x%x\n", status);
    exit(1);
  }

  // Restore the default framebuffer
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PickingTexture::updateTextureSize(unsigned int width,
                                       unsigned int height) {
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glBindTexture(GL_TEXTURE_2D, _pickingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, width, height, 0, GL_RGB_INTEGER,
               GL_UNSIGNED_INT, NULL);

  glBindTexture(GL_TEXTURE_2D, _depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void PickingTexture::enableWriting() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
}

void PickingTexture::disableWriting() {
  // Bind back the default framebuffer
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

PickingTexture::PixelInfo PickingTexture::readPixel(unsigned int x,
                                                    unsigned int y) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);

  glReadBuffer(GL_COLOR_ATTACHMENT0);

  PixelInfo pixel;
  float data[4];
  glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &pixel);
  glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, data);

  // spdlog::debug("{}, {}", data[0], data[1]);
  glReadBuffer(GL_NONE);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  return pixel;
}

}  // namespace ENDER
