#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string.h>
#include <stdexcept>

namespace Orasis {


    class Window {

        GLFWwindow* window;

        const int WIDTH;
        const int HEIGHT;

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
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            
            window = glfwCreateWindow(WIDTH, HEIGHT, window_title.c_str(), nullptr, nullptr);
        }
    };


}