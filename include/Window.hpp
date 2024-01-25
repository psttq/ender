#pragma once
#include <functional>
#include <unordered_map>

#include <glfw/glfw3.h>
#include <spdlog/spdlog.h>

namespace ENDER
{
    class Window
    {
        typedef std::function<void(int, int)> mousePosCallback;

        GLFWwindow *_window = nullptr;
        unsigned int _width;
        unsigned int _height;

        double _deltaTime = 0;
        double _lastFrame = 0;

        std::function<void(int, int)> _framebufferSizeCallback;

        std::unordered_map<int, mousePosCallback> _mousePosCallbacks;

        Window();

        ~Window()
        {
            spdlog::info("Deallocation window.");
            glfwTerminate();
        }

        void _posCursorCallback(GLFWwindow *window, double xpos, double ypos);

    public:
        static Window &instance()
        {
            static Window _instance;
            return _instance;
        }

        static int addMousePosCallback(mousePosCallback callback);

        static void deleteMousePosCallback(int key);

        static void setFramebufferSizeCallback(std::function<void(int, int)> framebufferSizeCallback);

        static void __framebufferSizeCallback(GLFWwindow *window, int width, int height);

        static void init(unsigned int width, unsigned int height);

        static bool windowShouldClose();

        static void swapBuffers();

        static void pollEvents();

        static int getHeight();

        static int getWidth();

        static void keyPressed(unsigned int key, std::function<void()> callBack);

        static void close();

        static void flash();

        static double deltaTime();

        static void enableCursor();
        static void disableCursor();


        GLFWwindow *getNativeWindow();
    };
} // namespace ENDER
