#pragma once
#include <Camera.hpp>
#include <glm/glm.hpp>

namespace ENDER {
    class FirstPersonCamera : public Camera{
        glm::vec3 _position{};
        glm::vec3 _front{0.f, 0.f, -1.f};
        glm::vec3 _up{0.f, 1.f, 0.f};

        float _speed = 2.5f;

        int _mousePosCallbackKey = -1;

        bool _firstCamera = true;
        double _lastX;
        double _lastY;

        double _yaw;
        double _pitch;

    public:
        FirstPersonCamera(const glm::vec3 &position);
        ~FirstPersonCamera();

        glm::mat4 getView() const override;

        void setSpeed(float speed);

        void proccessInput();
        void proccessMouseInput(double xpos, double ypos);

    };
} // namespace ENDER

