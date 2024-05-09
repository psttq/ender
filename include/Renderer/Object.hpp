#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "Material.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include <Shader.hpp>
#include <glm/glm.hpp>

namespace ENDER {
    class Object {
    public:
        enum class ObjectType {
            Surface,
            Line,
            Multi
        };

        bool isSelectable = false;

    protected:
        unsigned int _id;
        std::string _name;

        sptr<Shader> _shader = nullptr;

        sptr<VertexArray> _vertexArray = nullptr;
        Texture *_texture = nullptr;
        glm::vec3 _position{};
        glm::vec3 _rotation{};
        glm::vec3 _scale = glm::vec3(1.0f);

        sptr<Object> _childObject;

        bool _selected = false;

    public:

        Material material;

        ObjectType type = ObjectType::Surface;

        std::string label = "Object";

        Object(const std::string &name, sptr<VertexArray> vertexArray);

        Object(const std::string &name);
        ~Object(){
            spdlog::debug("deleted obj: {}", _id);
        }

        bool selected() const;

        void setSelected(bool selected);


        void setTexture(Texture *texture);

        Texture *getTexture() const;

        void setPosition(const glm::vec3 &position);

        void setRotation(const glm::vec3 &rotation);

        void setScale(const glm::vec3 &scale);

        glm::vec3 &getPosition();

        glm::vec3 &getRotation();

        glm::vec3 &getScale();

        glm::vec3 getPosition() const;

        glm::vec3 getRotation() const;

        glm::vec3 getScale() const;

        glm::mat4 getTransform() const;

        const std::string &getName();

        unsigned int getId() const;

        void setShader(sptr<Shader> shader);

        sptr<Shader> getShader();

        void setChildObject(sptr<Object> childObject);

        void resetChildObject();

        sptr<Object> getChildObject();

        sptr<VertexArray> getVertexArray() const;

        void setVertexArray(sptr<VertexArray> vertexArray);

        virtual void drawProperties();

        std::string getName() const;

        static sptr<Object> create(const std::string &name,
                                   sptr<VertexArray> vertexArray);

        static sptr<Object> createCube(const std::string &name);

        static sptr<Object> createGrid(const std::string &name);
    };

    static float CUBE_VERTICES[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f, -0.5f,
            -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 0.0f,
            0.0f, -1.0f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            1.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f,
            0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f,
            0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f,
            -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, -1.0f,
            0.0f, 0.0f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f,
            -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f,
            -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f,
            -1.0f, 0.0f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
            -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    static float GRID_VERTICES[] = {1, 1, 0, -1, -1, 0, -1, 1, 0,
                                    -1, -1, 0, 1, 1, 0, 1, -1, 0};
} // namespace ENDER
