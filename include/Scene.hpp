#pragma once
#include <vector>
#include <Object.hpp>
#include <glm/glm.hpp>
#include <Camera.hpp>

namespace ENDER
{
    class Scene
    {
        std::vector<Object *> _objects;
        Camera *_camera = nullptr;

    public:
        Scene();

        glm::mat4 calculateView();
        void addObject(Object *object);

        void setCamera(Camera *camera);
        Camera *getCamera();

        std::vector<Object *> &getObjects();
    };
} // namespace ENDER
