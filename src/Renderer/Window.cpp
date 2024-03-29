#include "GLFW/glfw3.h"
#include <../../3rd/spdlog/include/spdlog/spdlog.h>

#include <../../include/Renderer/Window.hpp>

ENDER::Window::Window() {}

void ENDER::Window::_posCursorCallback(GLFWwindow *window, double xpos,
                                       double ypos) {
  for (auto &func : _mousePosCallbacks) {
    func.second(xpos, ypos);
  }
  _mousePosition = {(int)xpos, (int)ypos};
}

void ENDER::Window::_scrollCallback(GLFWwindow *window, double offsetX,
                                    double offsetY) {
  for (auto &func : _scrollCallbacks) {
    func.second(offsetX, offsetY);
  }
}

void ENDER::Window::_clickCursorCallback(GLFWwindow *window, int button,
                                         int action, int mods) {
  MouseButton _button;
  switch (button) {
  case GLFW_MOUSE_BUTTON_RIGHT:
    _button = MouseButton::Right;
    break;
  case GLFW_MOUSE_BUTTON_LEFT:
    _button = MouseButton::Left;
    break;
  default:
    _button = MouseButton::None;
  }

  EventStatus _status = glfwActionToEventStatus(action);
  for (auto &func : _mouseClickCallbacks) {
    func.second(_button, _status);
  }
}

void ENDER::Window::_inputCallback(GLFWwindow *window, int key, int scancode,
                                   int action, int mods) {
  EventStatus _status = glfwActionToEventStatus(action);
  for (auto &func : _inputCallbacks) {
    func.second(key, _status);
  }
}

void ENDER::Window::init(unsigned int width, unsigned int height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
#endif
  instance()._window =
      glfwCreateWindow(width, height, "ENDER test", NULL, NULL);
  if (instance()._window == NULL) {
    spdlog::error("Failed to create GLFW window");
    glfwTerminate();
    throw;
  }
  glfwMakeContextCurrent(instance()._window);

  instance()._width = width;
  instance()._height = height;

  // glfwSetInputMode(instance()._window, GLFW_STICKY_KEYS, GLFW_TRUE);

  glfwSetCursorPosCallback(instance()._window,
                           [](GLFWwindow *window, double xpos, double ypos) {
                             instance()._posCursorCallback(window, xpos, ypos);
                           });
  glfwSetMouseButtonCallback(
      instance()._window,
      [](GLFWwindow *window, int button, int action, int mods) {
        instance()._clickCursorCallback(window, button, action, mods);
      });
  glfwSetKeyCallback(
      instance()._window,
      [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        instance()._inputCallback(window, key, scancode, action, mods);
      });
  glfwSetScrollCallback(instance()._window,
                        [](GLFWwindow *window, double offsetX, double offsetY) {
                          instance()._scrollCallback(window, offsetX, offsetY);
                        });
}

int ENDER::Window::addMousePosCallback(mousePosCallback callback) {
  int key = instance()._mousePosCallbacks.size();
  instance()._mousePosCallbacks.insert({key, callback});
  return key;
}

void ENDER::Window::deleteMousePosCallback(int key) {
  instance()._mousePosCallbacks.erase(key);
}

int ENDER::Window::addMouseClickCallback(mouseClickCallback callback) {
  int key = instance()._mousePosCallbacks.size();
  instance()._mouseClickCallbacks.insert({key, callback});
  return key;
}

void ENDER::Window::deleteMouseClickCallback(int key) {
  instance()._mouseClickCallbacks.erase(key);
}

int ENDER::Window::addInputCallback(inputCallback callback) {
  int key = instance()._inputCallbacks.size();
  instance()._inputCallbacks.insert({key, callback});
  return key;
}

void ENDER::Window::deleteInputCallback(int key) {
  instance()._inputCallbacks.erase(key);
}

int ENDER::Window::addMouseScrollCallback(mouseScrollCallback callback) {
  int key = instance()._inputCallbacks.size();
  instance()._scrollCallbacks.insert({key, callback});
  return key;
}

void ENDER::Window::deleteMouseScrollCallback(int key) {
  instance()._scrollCallbacks.erase(key);
}

void ENDER::Window::setFramebufferSizeCallback(
    std::function<void(int, int)> framebufferSizeCallback) {
  if (instance()._window == nullptr) {
    spdlog::error("First you should init Window.");
  }
  instance()._framebufferSizeCallback = framebufferSizeCallback;
  glfwSetFramebufferSizeCallback(instance()._window, __framebufferSizeCallback);
}

void ENDER::Window::__framebufferSizeCallback(GLFWwindow *window, int width,
                                              int height) {
  instance()._framebufferSizeCallback(width, height);
  instance()._width = width;
  instance()._height = height;
  spdlog::debug("Window resized. [width: {}, height: {}]", width, height);
}

bool ENDER::Window::windowShouldClose() {
  if (instance()._window == nullptr) {
    spdlog::error("First you should init Window.");
  }
  return glfwWindowShouldClose(instance()._window);
}

void ENDER::Window::swapBuffers() {
  if (instance()._window == nullptr) {
    spdlog::error("First you should init Window.");
  }
  glfwSwapBuffers(instance()._window);
}

void ENDER::Window::pollEvents() {
  if (instance()._window == nullptr) {
    spdlog::error("First you should init Window.");
  }
  glfwPollEvents();
}

int ENDER::Window::getHeight() {
  int width, height;
  glfwGetWindowSize(instance()._window, &width, &height);
  return height;
}

int ENDER::Window::getWidth() {
  int width, height;
  glfwGetWindowSize(instance()._window, &width, &height);
  return width;
}

glm::ivec2 ENDER::Window::getSize() {
  int width, height;
  glfwGetWindowSize(instance()._window, &width, &height);
  return {width, height};
}

void ENDER::Window::keyPressed(unsigned int key,
                               std::function<void()> callBack) {
  if (isKeyPressed(key)) {
    callBack();
  }
}

bool ENDER::Window::isKeyPressed(unsigned key) {
  return glfwGetKey(instance()._window, key) == GLFW_PRESS;
}

bool ENDER::Window::isKeyReleased(unsigned key) {
  return glfwGetKey(instance()._window, key) == GLFW_RELEASE;
}

void ENDER::Window::keyReleased(unsigned key, std::function<void()> callBack) {
  if (isKeyReleased(key)) {
    callBack();
  }
}

bool ENDER::Window::isMouseButtonPressed(const MouseButton &button) {
  switch (button) {
  case MouseButton::Left:
    return glfwGetMouseButton(instance()._window, GLFW_MOUSE_BUTTON_LEFT) ==
           GLFW_PRESS;
  case MouseButton::Right:
    return glfwGetMouseButton(instance()._window, GLFW_MOUSE_BUTTON_RIGHT) ==
           GLFW_PRESS;
  default:
    spdlog::error("ENDER::Window::isMouseButtonPressed: Unknown mouse button");
  }
  return false;
}

void ENDER::Window::mouseButtonPressed(const MouseButton &button,
                                       std::function<void()> callBack) {
  if (isMouseButtonPressed(button))
    callBack();
}

void ENDER::Window::close() {
  glfwSetWindowShouldClose(instance()._window, true);
}

void ENDER::Window::flash() {
  pollEvents();
  auto currentTime = glfwGetTime();
  instance()._deltaTime = currentTime - instance()._lastFrame;
  instance()._lastFrame = currentTime;
}

double ENDER::Window::deltaTime() { return instance()._deltaTime; }

double ENDER::Window::currentTime() { return glfwGetTime(); }

void ENDER::Window::enableCursor() {
  glfwSetInputMode(instance()._window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void ENDER::Window::disableCursor() {
  glfwSetInputMode(instance()._window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

GLFWwindow *ENDER::Window::getNativeWindow() { return _window; }

ENDER::Window::EventStatus
ENDER::Window::glfwActionToEventStatus(unsigned action) {
  EventStatus _status;
  switch (action) {
  case GLFW_PRESS:
    _status = EventStatus::Press;
    break;
  case GLFW_RELEASE:
    _status = EventStatus::Release;
    break;
  default:
    _status = EventStatus::None;
  }
  return _status;
}

glm::ivec2 ENDER::Window::getMousePosition() {
  return instance()._mousePosition;
}
