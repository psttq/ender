#include <OrthographicCamera.hpp>
#include <Window.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

ENDER::OrthographicCamera::OrthographicCamera(const glm::vec3 &position,
                                              const glm::vec2 &framebufferSize) : _position(position),
    _framebufferSize(framebufferSize) {
    _mousePosCallbackKey = Window::addMousePosCallback(
        [&](double xpos, double ypos) { proccessMouseInput(xpos, ypos); });
    _mouseClickCallbackKey = Window::addMouseClickCallback(
    [&](Window::MouseButton button, Window::EventStatus status) {
      if (button == Window::MouseButton::Right &&
          status == Window::EventStatus::Release) {
        _firstCamera = true;
      }
    });
}

ENDER::OrthographicCamera::~OrthographicCamera() {
    if (_mousePosCallbackKey != -1)
        Window::deleteMousePosCallback(_mousePosCallbackKey);
}

sptr<ENDER::OrthographicCamera> ENDER::OrthographicCamera::create(const glm::vec3 &position,
                                                                  const glm::vec2 &framebufferSize) {
    return std::make_shared<OrthographicCamera>(position, framebufferSize);
}

void ENDER::OrthographicCamera::setFramebufferSize(const glm::vec2 &size) {
    _framebufferSize = size;
}

void ENDER::OrthographicCamera::setActive(bool value) {
    _isActive = value;
}

bool ENDER::OrthographicCamera::isActive() const {
    return _isActive;
}

glm::mat4 ENDER::OrthographicCamera::getView() const {
    return glm::lookAt(_position, _position + _front, _up);
}

glm::mat4 ENDER::OrthographicCamera::getProjection() const {
    return glm::ortho(0.0f * _zoom, (float)_framebufferSize.x * _zoom, 0.0f * _zoom, (float)_framebufferSize.y * _zoom, 0.0f, 10.0f);
}

glm::vec3 ENDER::OrthographicCamera::getPosition() const {
    return _position;
}

glm::vec3 ENDER::OrthographicCamera::getFront() const {
    return _front;
}

bool ENDER::OrthographicCamera::getSpotlightToggled() const {
    return false;
}

void ENDER::OrthographicCamera::proccessMouseInput(double xpos, double ypos) {
    if(!_isActive)
        return;

    if(!Window::isMouseButtonPressed(Window::MouseButton::Right))
        return;

    if (_firstCamera) {
        _lastX = xpos;
        _lastY = ypos;
        _firstCamera = false;
    }

    int offsetX = xpos - _lastX;
    int offsetY = ypos - _lastY;

    _lastX = xpos;
    _lastY = ypos;

    // get as ratio +/- 1
    float dx = glm::abs((float) offsetX) / _framebufferSize.x;
    float dy = glm::abs((float) offsetY) / _framebufferSize.y;

    // now move camera by offset (might need to multiply by 2 here?)
    _position.x -= offsetX*dx*_speed;
    _position.z -= offsetY*dy*_speed;
}
