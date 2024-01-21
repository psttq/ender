#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ENDER::Renderer::Renderer()
{
    _shader = new Shader("../resources/shader.vs", "../resources/shader.fs");
    _shader->use();
    _shader->setInt("texture1", 0);
}

ENDER::Renderer::~Renderer()
{
    delete _shader;
}

void ENDER::Renderer::renderObject(Object *object)
{
    glm::mat4 view = glm::mat4(
        1.0f); // make sure to initialize matrix to identity matrix first
    glm::mat4 projection = glm::mat4(1.0f);
    projection =
        glm::perspective(glm::radians(45.0f),
                         (float)800 / (float)600, 0.1f, 100.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    // pass transformation matrices to the shader
    _shader->setMat4(
        "projection",
        projection); // note: currently we set the projection matrix each frame,
                     // but since the projection matrix rarely changes it's
                     // often best practice to set it outside the main loop only
                     // once.
    _shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, object->getPosition());
    // TODO: rotation
    //  model =
    //      glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    _shader->setMat4("model", model);

    object->getVertexArray()->bind();
    if (object->getTexture() != nullptr)
    {
        object->getTexture()->setAsCurrent();
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);
}