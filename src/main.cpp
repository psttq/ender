#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <Ender.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "PickingTexture.hpp"
#include "glm/gtc/constants.hpp"

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

unsigned int *generateGrid(int rows, int cols) {
    auto *indices = new unsigned int[(rows - 1) * (cols - 1) * 2 * 3];
    for (int row = 0; row < rows-1; row++) {
        for (int col = 0; col < cols - 1; col++) {
            unsigned int f = col + row * cols;
            unsigned int s1 = col + (row + 1) * cols;
            unsigned int s2 = s1 + 1;
            unsigned int t = f + 1;
            indices[(col+row*(cols-1))*6] = f;
            indices[(col+row*(cols-1))*6 + 1] = s1;
            indices[(col+row*(cols-1))*6 + 2] = s2;
            indices[(col+row*(cols-1))*6 + 3] = f;
            indices[(col+row*(cols-1))*6 + 4] = s2;
            indices[(col+row*(cols-1))*6 + 5] = t;
        }
    }
 return indices;
}


int main() {
    spdlog::set_level(spdlog::level::debug);

    float u_max = 2 * glm::pi<float>();
    float v_max = 2 * glm::pi<float>();

    int rows = 25;
    int cols = 25;

    auto *indices = generateGrid(rows, cols);


    for(auto i = 0; i < (rows - 1) * (cols - 1) * 2; i++) {
        printf("%d, %d, %d,", indices[i*3], indices[i*3+1], indices[i*3+2]);
    }

    float *vertices = new float[rows*cols*3];

    float h_u = u_max/(cols-1);
    float h_v = v_max/(rows-1);

    for(auto i = 0; i < rows; i++) {
        for(auto j = 0; j < cols; j++) {
            float u = h_u*j;
            float v = h_v*i;
            vertices[(j+i*cols)*3]=glm::sin(u)*glm::cos(v); // x
            vertices[(j+i*cols)*3+1]=glm::sin(u)*glm::sin(v); // x
            vertices[(j+i*cols)*3+2]=glm::cos(u); // x
            spdlog::debug("{} {}, {}, {}: {} {} {}", i, j, u, v, u*glm::cos(v), u*glm::sin(v), u);
        }
    }
    spdlog::debug("VERTICIES:");

    for(auto i = 0; i < rows*cols; i++) {
        printf("%f, %f, %f,", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    }


    ENDER::Window::init(SCR_WIDTH, SCR_HEIGHT);

    ENDER::Renderer::init();

    auto layout = new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}});
    auto vbo = new ENDER::VertexBuffer(layout);
    vbo->setData(vertices, sizeof(float)*rows*cols*3);

    auto ibo = new ENDER::IndexBuffer(indices, (rows - 1) * (cols - 1) * 2 * 3);

    auto vao = new ENDER::VertexArray();
    vao->addVBO(vbo);
    vao->setIndexBuffer(ibo);



    auto *lightCubeShader = new ENDER::Shader("../resources/lightShader.vs",
                                              "../resources/lightShader.fs");

    auto *scene = new ENDER::Scene();

    auto *camera = new ENDER::FirstPersonCamera({});

    scene->setCamera(camera);

    ENDER::Renderer::setClearColor({0.093f, 0.093f, 0.093f, 1.0f});

    ENDER::Window::addInputCallback([&](int key,
                                        ENDER::Window::EventStatus status) {
        if (key == GLFW_KEY_SPACE && status == ENDER::Window::EventStatus::Press) {
            auto pos = scene->getCamera()->getPosition();

            auto lightCube = ENDER::Object::createCube("Light Debug Cube");
            lightCube->setPosition(pos);
            lightCube->setScale(glm::vec3(0.2f));
            lightCube->setShader(lightCubeShader);
            scene->addObject(lightCube);

            auto pointLight = new ENDER::PointLight(pos, glm::vec3(1));
            scene->addLight(pointLight);
        }
        if(key == GLFW_KEY_L && status==ENDER::Window::EventStatus::Press) {
            ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Lines);
        }
        if(key == GLFW_KEY_K && status==ENDER::Window::EventStatus::Press) {
           ENDER::Renderer::setDrawType(ENDER::Renderer::DrawType::Triangles);
       }
    });

    glm::vec3 dir = {-0.2f, -1.0f, -0.3f};

    auto directionalLight = new ENDER::DirectionalLight(dir, {1, 1, 1});
    scene->addLight(directionalLight);

    auto grid = ENDER::Object::createGrid("Grid");
    // scene->addObject(grid);

    auto texture = new ENDER::Texture();
    texture->loadFromFile("../resources/textures/container.jpg", GL_RGB);

    auto object = new ENDER::Object("aea", vao);

    scene->addObject(object);

    while (!ENDER::Window::windowShouldClose()) {
        if (ENDER::Window::isMouseButtonPressed(ENDER::Window::MouseButton::Left)) {
            auto mousePosition = ENDER::Window::getMousePosition();
            auto pickedID =
                    ENDER::Renderer::pickObjAt(mousePosition.x, mousePosition.y);

            for (auto object: scene->getObjects()) {
                object->setSelected(object->getId() == pickedID);
            }
        }

        ENDER::Renderer::begin([&]() {
            ImGui::Text("FPS: %.2f", 1.0f / ENDER::Window::deltaTime());
            if (ImGui::SliderFloat3("Direciton", glm::value_ptr(dir), -1, 1)) {
                directionalLight->setDirection(dir);
            }

            for (auto obj: scene->getObjects()) {
                if (obj->selected())
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                ImGui::Text("%s", obj->getName().c_str());
                if (obj->selected())
                    ImGui::PopStyleColor();
            }
            for (auto light: scene->getLights()) {
                if (light->type == ENDER::Light::LightType::PointLight)
                    ImGui::Text("PointLight");
            }
        });

        ENDER::Window::keyPressed(GLFW_KEY_ESCAPE, [] { ENDER::Window::close(); });

        ENDER::Renderer::clear();
        ENDER::Renderer::clearPicking(); // FIXME: maybe unite this with clear()?

        ENDER::Renderer::renderScene(scene);

        ENDER::Renderer::end();

        camera->proccessInput();
    }

    delete scene;
    delete camera;
    delete lightCubeShader;

    return 0;
}
