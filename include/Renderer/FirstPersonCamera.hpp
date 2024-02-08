#pragma once
#include <Camera.hpp>
#include <glm/glm.hpp>
#include <ender_types.hpp>

namespace ENDER {
    class FirstPersonCamera : public Camera{
        glm::vec3 _position{};
        glm::vec3 _front{0.f, 0.f, -1.f};
        glm::vec3 _up{0.f, 1.f, 0.f};

        glm::vec2 _framebufferSize;

        float _defaultSpeed = 2.5f;
        float _shiftSpeed = 5.f;

        float _currentSpeed = _defaultSpeed;

        int _mousePosCallbackKey = -1;
        int _mouseClickCallbackKey = -1;
        int _inputCallbackKey = -1;

        bool _isActive = false;

        bool _firstCamera = true;
        double _lastX;
        double _lastY;

        double _yaw{};
        double _pitch{};

        bool _spotLigthToggled = false;

    public:
        FirstPersonCamera(const glm::vec3 &position, const glm::vec2 &framebufferSize);
        ~FirstPersonCamera();

        static sptr<FirstPersonCamera> create(const glm::vec3 &position, const glm::vec2 &framebufferSize);

        glm::mat4 getView() const override;
        glm::mat4 getProjection() const override;

        void setSpeed(float speed);

        void setFramebufferSize(const glm::vec2 &size);

        void setActive(bool value);
        bool isActive() const;

        glm::vec3 getPosition() const override;
        glm::vec3 getFront() const override;

        bool getSpotlightToggled() const override;

        void proccessInput();
        void proccessMouseInput(double xpos, double ypos);

    };
} // namespace ENDER

