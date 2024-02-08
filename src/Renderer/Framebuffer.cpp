#include <Framebuffer.hpp>
#include <memory>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

ENDER::Framebuffer::Framebuffer(float width, float height) {
    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);

    glGenTextures(1, &_tid);
    glBindTexture(GL_TEXTURE_2D, _tid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tid, 0);

    glGenRenderbuffers(1, &_rid);
    glBindRenderbuffer(GL_RENDERBUFFER, _rid);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rid);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        spdlog::error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    _width = width;
    _height = height;
}

ENDER::Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &_id);
    glDeleteTextures(1, &_tid);
    glDeleteRenderbuffers(1, &_rid);
}

sptr<ENDER::Framebuffer> ENDER::Framebuffer::create(float width, float height) {
    return std::shared_ptr<Framebuffer>(new Framebuffer(width, height));
}

uint ENDER::Framebuffer::getId() {
    return _id;
}

uint ENDER::Framebuffer::getTextureId() {
    return _tid;
}

void ENDER::Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void ENDER::Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ENDER::Framebuffer::rescale(float width, float height) {
    glBindTexture(GL_TEXTURE_2D, _tid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tid, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, _rid);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rid);

    _width = width;
    _height = height;
}
