#pragma once
#include <vector>
#include <Object.hpp>
#include <glm/glm.hpp>
#include <Camera.hpp>
#include <Light.hpp>

namespace ENDER
{
    class Scene
    {
        std::vector<Object *> _objects;
        Camera *_camera = nullptr;
        std::vector<Light *> _lights;

    public:
        Scene();

        glm::mat4 calculateView() const;
        void addObject(Object *object);

        void setCamera(Camera *camera);
        Camera *getCamera();

        void addLight(Light *light);

        const std::vector<Light *> &getLights();

        std::vector<Object *> &getObjects();
    };
} // namespace ENDER
