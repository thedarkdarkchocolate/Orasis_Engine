#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Window.hpp"
#include "Render.hpp"
#include "GameObject.hpp"
#include "RenderSystem.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {


    class App {

        // -------- MEMBER VARIABLES -------- //

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Window ors_Window{WIDTH, HEIGHT, "AAApp!!"};
        Device ors_Device{ors_Window};
        Render ors_Render{ors_Window, ors_Device};

        std::vector<GameObject> gameObjects;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        App()
        {
            loadGameObjects();
        }

        ~App()
        {}

        App(const App&) = delete;
        App &operator=(const App&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        void run() {

            RenderSystem renderSys{ors_Device, ors_Render.getSwapChainRenderPass()};    

            while(!ors_Window.shouldClose())
            {
                glfwPollEvents();
                
                if (VkCommandBuffer cmndBuffer = ors_Render.beginFrame())
                {
                    ors_Render.startSwapChainRenderPass(cmndBuffer);
                    renderSys.renderGameObjects(cmndBuffer, gameObjects);
                    ors_Render.endSwapChainRenderPass(cmndBuffer);
                    ors_Render.endFrame();

                }
                
            }

            vkDeviceWaitIdle(ors_Device.device());

        }

        private:

        // temporary helper function, creates a 1x1x1 cube centered at offset
        std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset) {
                std::vector<Model::Vertex> vertices{
            
                    // left face (white)
                    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
                    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
                    {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
                    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
                    {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
                    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            
                    // right face (yellow)
                    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
                    {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
                    {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
                    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
                    {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
                    {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            
                    // top face (orange, remember y axis points down)
                    {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                    {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                    {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                    {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            
                    // bottom face (red)
                    {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                    {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
                    {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
                    {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                    {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                    {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            
                    // nose face (blue)
                    {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                    {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                    {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                    {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                    {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                    {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            
                    // tail face (green)
                    {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                    {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                    {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                    {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                    {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                    {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            
                };

            for (auto& v : vertices) 
                v.position += offset;
            
            return std::make_unique<Model>(device, vertices);
        }

        void loadGameObjects()
        {
            std::shared_ptr<Model> model = createCubeModel(ors_Device, {0.f, 0.f, 0.f});

            GameObject cube = GameObject::createGameObject();

            cube.model = model;
            cube.transform.translation = {0.f, 0.f, 0.5f};
            cube.transform.scale = {0.5f, 0.5f, 0.5f};
            gameObjects.push_back(std::move(cube));
        }



    };


}