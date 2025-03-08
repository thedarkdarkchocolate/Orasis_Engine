#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string.h>
#include <stdexcept>

namespace Orasis {


    class Window {

        GLFWwindow* window;

        int WIDTH;
        int HEIGHT;
        bool frameBufferResized = false;

        std::string window_title;
        

        public:

        Window(int width, int height, std::string title)
        : WIDTH(width), HEIGHT(height), window_title(title)
        {
            initWindow();
        }

        Window(const Window&) = delete;
        Window &operator=(const Window&) = delete;

        ~Window()
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        bool shouldClose () 
        {
            return glfwWindowShouldClose(window);
        }

        bool wasWindowResized()
        {
            return frameBufferResized;
        }

        void resetWindowResizedFlag()
        {
            frameBufferResized = false;
        }

        VkExtent2D getExtent()
        {
            return {static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT)};
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
        {
            if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
                throw std::runtime_error("failed to create window surface");
        }

        private:

        void initWindow(){

            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            
            window = glfwCreateWindow(WIDTH, HEIGHT, window_title.c_str(), nullptr, nullptr);

            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
            
        }

        static void frameBufferResizedCallback(GLFWwindow* window, int width, int height)
        {
            Window* ors_Window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

            ors_Window->frameBufferResized = true;
            ors_Window->WIDTH = width;
            ors_Window->HEIGHT = height;

        }

    };


}