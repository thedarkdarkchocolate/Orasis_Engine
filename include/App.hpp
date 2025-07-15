#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Camera.hpp"
#include "Window.hpp"
#include "Render.hpp"
#include "GameObject.hpp"
#include "RenderSystem.hpp"
#include "Kmb_movement_controller.hpp"

#include <memory>
#include <chrono>
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

                Camera camera{};
                KmbMovementController cameraController{};
                GameObject cameraObj = GameObject::createGameObject();

                auto currTime = std::chrono::high_resolution_clock::now();


                
                while(!ors_Window.shouldClose())
                {
                    glfwPollEvents();
                    
                    // Time step
                    auto newTime = std::chrono::high_resolution_clock::now();
                    float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currTime).count();
                    currTime = newTime;
                    
                    
                    float aspect = ors_Render.getAspectRatio();
                    cameraController.moveInPlaneXZ(ors_Window.getWindow(), cameraObj, dt);
                    camera.setViewYXZ(cameraObj.transform.translation, cameraObj.transform.rotation);

                    camera.setPrespectiveProjection(glm::radians(60.f), aspect, 0.1f, 100.f);
                    
                    if (VkCommandBuffer cmndBuffer = ors_Render.beginFrame())
                    {
                        ors_Render.startSwapChainRenderPass(cmndBuffer);
                        renderSys.renderGameObjects(cmndBuffer, gameObjects, camera);
                        ors_Render.endSwapChainRenderPass(cmndBuffer);
                        ors_Render.endFrame();

                    }
                    

                    
                }

                vkDeviceWaitIdle(ors_Device.device());

            }

        private:

            // temporary helper function, creates a 1x1x1 cube centered at offset
            std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset) {

                Model::Builder modelBuilder{};

                modelBuilder.vertices = {
                    // left face (white)
                    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},      //1     
                    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},        //2   
                    {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},       //3     
                    {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},       //4     
                
                    // right face (yellow)
                    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},       //5     
                    {{.5f, .5f, .5f}, {.8f, .8f, .1f}},         //6     
                    {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},        //7     
                    {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},        //8     
                
                    // top face (orange, remember y axis points down)
                    {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},      //9     
                    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},        //10     
                    {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},       //11    
                    {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},       //12     
                
                    // bottom face (red)
                    {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},       //13     
                    {{.5f, .5f, .5f}, {.8f, .1f, .1f}},         //14     
                    {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},        //15     
                    {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},        //16     
                
                    // nose face (blue)
                    {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},      //17     
                    {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},        //18     
                    {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},       //19     
                    {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},       //20     
                
                    // tail face (green)
                    {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},     //21     
                    {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},       //22     
                    {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},      //23     
                    {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},      //24     
                };

                if (glm::dot(offset, offset) > std::numeric_limits<float>::epsilon())
                    for (auto& v : modelBuilder.vertices) {
                        v.position += offset;
                    }
                
                modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                        12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
                
                return std::make_unique<Model>(device, modelBuilder);
            }

            void loadGameObjects()
            {
                std::shared_ptr<Model> model = createCubeModel(ors_Device, {0.f, 0.f, 0.f});

                for (int i = -1; i < 2; i++)
                {
                    if(!i) continue;

                    GameObject cube = GameObject::createGameObject();
                    
                    cube.model = model;
                    cube.transform.translation = {-i, 0.f, 2.5f};
                    cube.transform.scale = {0.5f, 0.5f, 0.5f};
                    gameObjects.push_back(std::move(cube));
                }
            }



    };


}