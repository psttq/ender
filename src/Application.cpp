#include <Application.hpp>

namespace ENDER {

Application::Application(uint appWidth, uint appHeight) {
  ENDER::Window::init(appWidth, appHeight);
  ENDER::Renderer::init();

  _keyCallbackKey = ENDER::Window::addInputCallback([&](int key, Window::EventStatus status){
    onKey(key, status);
    if(status == Window::EventStatus::Press){
      onKeyPress(key);
    }
    else if(status == Window::EventStatus::Release){
      onKeyRelease(key);
    }
  });

  _mouseMoveCallbackKey = ENDER::Window::addMousePosCallback([&](uint x, uint y){
    onMouseMove(x, y);
  });
  _mouseClickCallbackKey = ENDER::Window::addMouseClickCallback([&](Window::MouseButton button, Window::EventStatus status){
    onMouseClick(button, status);
  });
}

void Application::_render(){
  Renderer::begin([&]{onGUI();}); 
  render();
  Renderer::end();
}

void Application::close(){
  _isRunning = false;
}

Application::~Application(){
  ENDER::Window::deleteInputCallback(_keyCallbackKey);
  ENDER::Window::deleteMousePosCallback(_mouseMoveCallbackKey);
  ENDER::Window::deleteMouseClickCallback(_mouseClickCallbackKey);
}

void Application::start(){
  _isRunning = true;
  onStart();
  while(_isRunning && !Window::windowShouldClose()){
    update(Window::deltaTime());
    _render();
  }
  onClose();
  Window::close();
}

} // namespace ENDER
