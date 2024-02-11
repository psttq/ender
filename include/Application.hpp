#pragma once
#include <Ender.hpp>

namespace ENDER {

class Application {
protected:
  bool _isRunning = false;

  bool _mouseMoveCallbackKey;
  bool _keyCallbackKey;
  bool _mouseClickCallbackKey;

  void _render();

public:
  Application(uint appWidth, uint appHeight);
  virtual ~Application();

  void start();
  void close();

  virtual void onGUI() = 0;

  virtual void onStart() = 0;
  virtual void onClose() = 0;

  virtual void update(float deltaTime) = 0;
  virtual void render() = 0;

  virtual void onMouseMove(uint x, uint y) = 0;
  virtual void onKeyPress(int key) = 0;
  virtual void onKeyRelease(int key) = 0;
  virtual void onKey(int key, Window::EventStatus status) = 0;
  virtual void onMouseClick(Window::MouseButton button,
                            Window::EventStatus status) = 0;
};

} // namespace ENDER
