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
#include "Buffer.hpp"

#include <memory>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {

    struct UBO_struct {
        glm::mat4 projectionView{1.f};
        glm::vec3 lightPos = {1.f, -3.5, -1.f};
    };



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

                std::vector<std::unique_ptr<Buffer>> uniformBuffers (SwapChain::MAX_FRAMES_IN_FLIGHT);

                for (int i = 0; i < uniformBuffers.size(); i++)
                {
                    // Uniform Buffer
                    uniformBuffers[i] = std::make_unique<Buffer> (
                        ors_Device,
                        sizeof(UBO_struct),
                        1,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                        ors_Device.properties.limits.minUniformBufferOffsetAlignment
                    );
                    uniformBuffers[i]->map();    // Enables writing on the buffer
                }

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
                        int frameIndex = ors_Render.getFrameIndex();

                        // Update Frame Info
                        FrameInfo frameInfo {
                            cmndBuffer,
                            camera,
                            frameIndex,
                            dt
                        };

                        // Update UBO
                        UBO_struct ubo_s{};
                        ubo_s.projectionView = camera.getProjection() * camera.getViewMatrix();
                        
                        uniformBuffers[frameIndex]->writeToBuffer(&ubo_s);
                        uniformBuffers[frameIndex]->flush(frameIndex);

                        // Render
                        
                        ors_Render.startSwapChainRenderPass(cmndBuffer);
                        renderSys.renderGameObjects(frameInfo, gameObjects);
                        ors_Render.endSwapChainRenderPass(cmndBuffer);
                        ors_Render.endFrame();

                    }
                    

                    
                }

                vkDeviceWaitIdle(ors_Device.device());

            }

        private:

            void loadGameObjects()
            {
                std::shared_ptr<Model> model = Model::createModelFromFile(ors_Device, "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/models/flat_vase.obj");
                
                
                GameObject gameObj = GameObject::createGameObject();
                
                gameObj.model = model;
                gameObj.transform.translation = {0.f, 0.5f, 2.5f};
                gameObj.transform.scale = glm::vec3(3.f);
                gameObjects.push_back(std::move(gameObj));
                
                model = Model::createModelFromFile(ors_Device, "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/models/cube.obj");
                gameObj = GameObject::createGameObject();
                
                gameObj.model = model;
                gameObj.transform.translation = {1.f, -3.5, -1.f};
                gameObj.transform.scale = glm::vec3(0.05f);
                gameObjects.push_back(std::move(gameObj));
                
            }



    };


}