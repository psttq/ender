#pragma once
#include <glfw/glfw3.h>
#include <functional>
#include <spdlog/spdlog.h>

namespace ENDER
{
    class Window
    {
        GLFWwindow *_window = nullptr;
        unsigned int _width;
        unsigned int _height;

        std::function<void(int, int)> _framebufferSizeCallback;

        Window();

        ~Window()
        {
            spdlog::info("Deallocation window.");
            glfwTerminate();
        }

    public:
        static Window &instance()
        {
            static Window _instance;
            return _instance;
        }

        static void setFramebufferSizeCallback(std::function<void(int, int)> framebufferSizeCallback);

        static void __framebufferSizeCallback(GLFWwindow *window, int width, int height);

        static void init(unsigned int width, unsigned int height);

        static bool windowShouldClose();

        static void swapBuffers();

        static void pollEvents();

        static int getHeight();

        static int getWidth();

        GLFWwindow *getNativeWindow();
    };
} // namespace ENDER
