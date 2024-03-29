#include <FirstPersonCamera.hpp>
#include <Window.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

ENDER::FirstPersonCamera::FirstPersonCamera(const glm::vec3 &position, const glm::vec2 &framebufferSize)
    : _position(position), _framebufferSize(framebufferSize) {
  _mousePosCallbackKey = Window::addMousePosCallback(
      [&](double xpos, double ypos) { proccessMouseInput(xpos, ypos); });
  _mouseClickCallbackKey = Window::addMouseClickCallback(
      [&](Window::MouseButton button, Window::EventStatus status) {
        if (button == Window::MouseButton::Right &&
            status == Window::EventStatus::Release) {
          Window::enableCursor();
          _firstCamera = true;
        }
      });

  _inputCallbackKey =
      Window::addInputCallback([&](int key, Window::EventStatus status) {
        if (key == GLFW_KEY_E && status == Window::EventStatus::Release)
          Window::enableCursor();
        _firstCamera = true;
      });
}

sptr<ENDER::FirstPersonCamera> ENDER::FirstPersonCamera::create(const glm::vec3 &position, const glm::vec2 &framebufferSize)
{
  return std::make_shared<FirstPersonCamera>(position, framebufferSize);
}

ENDER::FirstPersonCamera::~FirstPersonCamera() {
  if (_mousePosCallbackKey != -1)
    Window::deleteMousePosCallback(_mousePosCallbackKey);
  if (_mouseClickCallbackKey != -1)
    Window::deleteMouseClickCallback(_mouseClickCallbackKey);
  if(_inputCallbackKey != -1)
    Window::deleteInputCallback(_inputCallbackKey);
}

glm::mat4 ENDER::FirstPersonCamera::getView() const {
  return glm::lookAt(_position, _position + _front, _up);
}

glm::mat4 ENDER::FirstPersonCamera::getProjection() const {
  return glm::perspective(
        glm::radians(45.0f),
        (float) _framebufferSize.x / (float) _framebufferSize.y, 0.1f, 100.0f);
}

void ENDER::FirstPersonCamera::setSpeed(float speed) { _defaultSpeed = speed; }

void ENDER::FirstPersonCamera::setFramebufferSize(const glm::vec2 &size) {
  _framebufferSize = size;
}

void ENDER::FirstPersonCamera::setActive(bool value) {
  _isActive = value;
}

bool ENDER::FirstPersonCamera::isActive() const {
  return _isActive;
}

glm::vec3 ENDER::FirstPersonCamera::getPosition() const { return _position; }

glm::vec3 ENDER::FirstPersonCamera::getFront() const { return _front; }

bool ENDER::FirstPersonCamera::getSpotlightToggled() const {
  return _spotLigthToggled;
}

void ENDER::FirstPersonCamera::proccessInput() {
  if(!_isActive)
    return;
  float __speed = _currentSpeed * static_cast<float>(Window::deltaTime());
  Window::keyPressed(GLFW_KEY_W, [&] { _position += __speed * _front; });
  Window::keyPressed(GLFW_KEY_S, [&] { _position -= __speed * _front; });
  Window::keyPressed(GLFW_KEY_A, [&] {
    _position -= glm::normalize(glm::cross(_front, _up)) * __speed;
  });
  Window::keyPressed(GLFW_KEY_D, [&] {
    _position += glm::normalize(glm::cross(_front, _up)) * __speed;
  });
  Window::keyPressed(GLFW_KEY_X, [&] { _position += _up * __speed; });
  Window::keyPressed(GLFW_KEY_Z, [&] { _position -= _up * __speed; });
  Window::keyPressed(GLFW_KEY_F,
                     [&] { _spotLigthToggled = !_spotLigthToggled; });

  Window::keyPressed(GLFW_KEY_LEFT_SHIFT, [&] { _currentSpeed = _shiftSpeed; });
  Window::keyReleased(GLFW_KEY_LEFT_SHIFT,
                      [&] { _currentSpeed = _defaultSpeed; });
}

void ENDER::FirstPersonCamera::proccessMouseInput(double xpos, double ypos) {
  if(!_isActive)
    return;

  if (!Window::isMouseButtonPressed(Window::MouseButton::Right) &&
      !Window::isKeyPressed(GLFW_KEY_E))
    return;

  Window::disableCursor();

  if (_firstCamera) {
    _lastX = xpos;
    _lastY = ypos;
    _firstCamera = false;
  }

  float xoffset = xpos - _lastX;
  float yoffset = _lastY - ypos;
  _lastX = xpos;
  _lastY = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  _yaw += xoffset;
  _pitch += yoffset;

  if (_pitch > 89.0f)
    _pitch = 89.0f;
  if (_pitch < -89.0f)
    _pitch = -89.0f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  direction.y = sin(glm::radians(_pitch));
  direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  _front = glm::normalize(direction);
}
