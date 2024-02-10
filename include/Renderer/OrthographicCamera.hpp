#pragma once
#include <Camera.hpp>
#include <ender_types.hpp>
#include <glm/glm.hpp>

namespace ENDER{
    class OrthographicCamera : public Camera{
        glm::vec3 _position{0, 4, 0};
        glm::vec3 _front{0.f, -100.f, -1.f};
        glm::vec3 _up{0.f, 1.f, 0.f};

        int _mousePosCallbackKey = -1;
        int _mouseClickCallbackKey = -1;
        int _inputCallbackKey = -1;

        bool _isActive = false;
        bool _firstCamera = true;

        double _lastX;
        double _lastY;

        float _zoom = 1.f/130.f;
        float _speed = 0.3f;

        glm::vec2 _framebufferSize;
    public:

        OrthographicCamera(const glm::vec3 &position, const glm::vec2 &framebufferSize);
        ~OrthographicCamera();

        static sptr<OrthographicCamera> create(const glm::vec3 &position, const glm::vec2 &framebufferSize);

        void setFramebufferSize(const glm::vec2 &size);

        void setActive(bool value);
        bool isActive() const;

        glm::mat4 getView() const override;
        glm::mat4 getProjection() const override;
        glm::vec3 getPosition() const override;
        glm::vec3 getFront() const override;

        bool getSpotlightToggled() const override;


        void proccessMouseInput(double xpos, double ypos);


    };
};
