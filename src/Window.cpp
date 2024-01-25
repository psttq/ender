#include <Window.hpp>
#include <spdlog/spdlog.h>

ENDER::Window::Window()
{

}

void ENDER::Window::_posCursorCallback(GLFWwindow *window, double xpos, double ypos) {
    for(auto &func: _mousePosCallbacks) {
        func.second(xpos, ypos);
    }
}

void ENDER::Window::init(unsigned int width, unsigned int height)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    instance()._window =
        glfwCreateWindow(width, height, "ENDER test", NULL, NULL);
    if (instance()._window == NULL)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        throw;
    }
    glfwMakeContextCurrent(instance()._window);

    instance()._width = width;
    instance()._height = height;

    glfwSetCursorPosCallback(instance()._window, [](GLFWwindow *window, double xpos, double ypos) {
        instance()._posCursorCallback(window, xpos, ypos);
    });
}

int ENDER::Window::addMousePosCallback(mousePosCallback callback) {
    int key = instance()._mousePosCallbacks.size();
    instance()._mousePosCallbacks.insert({key, callback});
    return key;
}

void ENDER::Window::deleteMousePosCallback(int key) {
    instance()._mousePosCallbacks.erase(key);
}

void ENDER::Window::setFramebufferSizeCallback(std::function<void(int, int)> framebufferSizeCallback)
{
    if (instance()._window == nullptr)
    {
        spdlog::error("First you should init Window.");
    }
    instance()._framebufferSizeCallback = framebufferSizeCallback;
    glfwSetFramebufferSizeCallback(instance()._window, __framebufferSizeCallback);
}

void ENDER::Window::__framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    instance()._framebufferSizeCallback(width, height);
    instance()._width = width;
    instance()._height = height;
    spdlog::debug("Window resized. [width: {}, height: {}]", width, height);
}

bool ENDER::Window::windowShouldClose()
{
    if (instance()._window == nullptr)
    {
        spdlog::error("First you should init Window.");
    }
    return glfwWindowShouldClose(instance()._window);
}

void ENDER::Window::swapBuffers()
{
    if (instance()._window == nullptr)
    {
        spdlog::error("First you should init Window.");
    }
    glfwSwapBuffers(instance()._window);
}

void ENDER::Window::pollEvents()
{
    if (instance()._window == nullptr)
    {
        spdlog::error("First you should init Window.");
    }
    glfwPollEvents();
}

int ENDER::Window::getHeight()
{
    return instance()._height;
}

int ENDER::Window::getWidth()
{
    return instance()._width;
}

void ENDER::Window::keyPressed(unsigned int key, std::function<void()> callBack) {
    if (glfwGetKey(instance()._window, key) == GLFW_PRESS) {
        callBack();
    }
}

void ENDER::Window::close() {
    glfwSetWindowShouldClose(instance()._window, true);
}

void ENDER::Window::flash() {
    pollEvents();
    auto currentTime = glfwGetTime();
    instance()._deltaTime = currentTime - instance()._lastFrame;
    instance()._lastFrame = currentTime;
}

double ENDER::Window::deltaTime() {
    return instance()._deltaTime;
}

void ENDER::Window::enableCursor() {
    glfwSetInputMode(instance()._window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void ENDER::Window::disableCursor() {
    glfwSetInputMode(instance()._window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

GLFWwindow *ENDER::Window::getNativeWindow()
{
    return _window;
}
