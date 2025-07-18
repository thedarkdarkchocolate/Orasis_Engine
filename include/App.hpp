#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Render.hpp"
#include "GameObject.hpp"
#include "RenderSystem.hpp"
#include "Kmb_movement_controller.hpp"
#include "Descriptors.hpp"

#include <memory>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {

    struct UBO_struct {
        alignas(16) glm::mat4 projectionView{1.f};
        alignas(16) glm::vec3 lightPos = {1.f, -3.5, -1.f};
    };



    class App {

        // -------- MEMBER VARIABLES -------- //

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Window ors_Window{WIDTH, HEIGHT, "AAApp!!"};
        Device ors_Device{ors_Window};
        Render ors_Render{ors_Window, ors_Device};

        // Order of decleration matters (Variable get created from top to bottom and destroyed in the reverse)
        // we need the global pool to be destroyed before the device
        std::unique_ptr<DescriptorPool> globalPool{};
        std::vector<GameObject> gameObjects;
        

        // -------- -------- -------- -------- //


        public:

            // -------- CONSTRUCTOR etc -------- //

            App()
            {
                
                // Order of method chain is from top to bottom
                globalPool = 
                    DescriptorPool::Builder(ors_Device)                                         
                        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                        .build();
                
                loadGameObjects();
            }

            ~App()
            {}

            App(const App&) = delete;
            App &operator=(const App&) = delete;

            // -------- -------- -------- -------- //





            // -------- FUNCTIONS -------- //

            void run() {

                // Creating a total number of buffers as the number of frames in flight (each for every frame) 
                std::vector<std::unique_ptr<Buffer>> uniformBuffers (SwapChain::MAX_FRAMES_IN_FLIGHT);

                for (int i = 0; i < uniformBuffers.size(); i++)
                {
                    // Uniform Buffer
                    uniformBuffers[i] = std::make_unique<Buffer> (
                        ors_Device,
                        sizeof(UBO_struct),
                        1,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,        // Need to use this bit if we dont manually flush the buffers to the device VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                        ors_Device.properties.limits.minUniformBufferOffsetAlignment
                    );
                    uniformBuffers[i]->map();    // Enables writing on the buffer
                }

                // Configuring Descriptor Layout Info
                auto gloabalSetLayout = DescriptorSetLayout::Builder(ors_Device)
                                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                        .build();
                
                std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

                for(int i = 0; i < globalDescriptorSets.size(); i ++)
                {
                    auto bufferInfo = uniformBuffers[i]->descriptorInfo();
                    DescriptorWriter(*gloabalSetLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .build(globalDescriptorSets[i]);
                }

                RenderSystem renderSys{ors_Device, ors_Render.getSwapChainRenderPass(), gloabalSetLayout->getDescriptorSetLayout()};   

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
                            globalDescriptorSets[frameIndex],
                            frameIndex,
                            dt
                        };

                        // Update UBO
                        UBO_struct ubo_s{};
                        ubo_s.projectionView = camera.getProjection() * camera.getViewMatrix();
                        
                        uniformBuffers[frameIndex]->writeToBuffer(&ubo_s);
                        uniformBuffers[frameIndex]->flush();

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