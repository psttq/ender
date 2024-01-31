#pragma once
#include <functional>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "glm/vec2.hpp"

namespace ENDER
{
    class Window
    {
    public:
        enum class MouseButton {
            Left,
            Right,
            None
        };

        enum class EventStatus {
            Release,
            Press,
            None
        };

    private:

        typedef std::function<void(int, int)> mousePosCallback;
        typedef std::function<void(MouseButton, EventStatus)> mouseClickCallback;
        typedef std::function<void(int, EventStatus)> inputCallback;


        GLFWwindow *_window = nullptr;
        unsigned int _width;
        unsigned int _height;

        double _deltaTime = 0;
        double _lastFrame = 0;

        std::function<void(int, int)> _framebufferSizeCallback;

        std::unordered_map<int, mousePosCallback> _mousePosCallbacks;
        std::unordered_map<int, mouseClickCallback> _mouseClickCallbacks;
        std::unordered_map<int, inputCallback> _inputCallbacks;
        glm::ivec2 _mousePosition;


        Window();

        ~Window()
        {
//            spdlog::info("Deallocation window.");
            glfwTerminate();
        }

        void _posCursorCallback(GLFWwindow *window, double xpos, double ypos);
        void _clickCursorCallback(GLFWwindow* window, int button, int action, int mods);
        void _inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    public:


        static Window &instance()
        {
            static Window _instance;
            return _instance;
        }

        static int addMousePosCallback(mousePosCallback callback);

        static void deleteMousePosCallback(int key);

        static int addMouseClickCallback(mouseClickCallback callback);

        static void deleteMouseClickCallback(int key);

        static int addInputCallback(inputCallback callback);

        static  void deleteInputCallback(int key);

        static void setFramebufferSizeCallback(std::function<void(int, int)> framebufferSizeCallback);

        static void __framebufferSizeCallback(GLFWwindow *window, int width, int height);

        static void init(unsigned int width, unsigned int height);

        static bool windowShouldClose();

        static void swapBuffers();

        static void pollEvents();

        static int getHeight();

        static int getWidth();

        static glm::ivec2 getSize();

        static void keyPressed(unsigned int key, std::function<void()> callBack);

        static bool isKeyPressed(unsigned int key);

        static bool isKeyReleased(unsigned int key);

        static void keyReleased(unsigned int key, std::function<void()> callBack);

        static glm::ivec2 getMousePosition();

        static bool isMouseButtonPressed(const MouseButton &button);

        static void mouseButtonPressed(const MouseButton &button, std::function<void()> callBack);

        static void close();

        static void flash();

        static double deltaTime();

        static void enableCursor();
        static void disableCursor();

        GLFWwindow *getNativeWindow();

    private:
        EventStatus glfwActionToEventStatus(unsigned int action);

    };
} // namespace ENDER
