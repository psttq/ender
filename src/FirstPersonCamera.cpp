#include <FirstPersonCamera.hpp>
#include <Window.hpp>
#include <glm/ext/matrix_transform.hpp>

ENDER::FirstPersonCamera::FirstPersonCamera(const glm::vec3 &position) : _position(position) {
    _mousePosCallbackKey = Window::addMousePosCallback([&](double xpos, double ypos) { proccessMouseInput(xpos, ypos); });
}

ENDER::FirstPersonCamera::~FirstPersonCamera() {
    if(_mousePosCallbackKey == -1)
        return;
    Window::deleteMousePosCallback(_mousePosCallbackKey);
}

glm::mat4 ENDER::FirstPersonCamera::getView() const {
    return glm::lookAt(_position, _position + _front, _up);
}

void ENDER::FirstPersonCamera::setSpeed(float speed) {
    _speed = speed;
}

void ENDER::FirstPersonCamera::proccessInput() {
    const float __speed = _speed * static_cast<float>(Window::deltaTime());
    Window::keyPressed(GLFW_KEY_W, [&] { _position += __speed * _front; });
    Window::keyPressed(GLFW_KEY_S, [&] { _position -= __speed * _front; });
    Window::keyPressed(GLFW_KEY_A, [&] { _position -= glm::normalize(glm::cross(_front, _up)) * __speed; });
    Window::keyPressed(GLFW_KEY_D, [&] { _position += glm::normalize(glm::cross(_front, _up)) * __speed; });
}

void ENDER::FirstPersonCamera::proccessMouseInput(double xpos, double ypos) {
    if (_firstCamera)
    {
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

    _yaw   += xoffset;
    _pitch += yoffset;

    if(_pitch > 89.0f)
        _pitch = 89.0f;
    if(_pitch < -89.0f)
        _pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    direction.y = sin(glm::radians(_pitch));
    direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _front = glm::normalize(direction);
}
