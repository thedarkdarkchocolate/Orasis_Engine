#pragma once

#include "Header_Includes/App_Header.hpp"

namespace Orasis {



    class App {

        // -------- MEMBER VARIABLES -------- //

        static constexpr int WIDTH = 1000;
        static constexpr int HEIGHT = 800;

        Window ors_Window{WIDTH, HEIGHT, "Orasis Engine"};
        Device ors_Device{ors_Window};
        Render ors_Render{ors_Window, ors_Device};
        std::unique_ptr<UI> ui;


        // Order of decleration matters (Variable get created from top to bottom and destroyed in the reverse)
        // we need the global pool to be destroyed before the device
        std::unique_ptr<DescriptorPool> globalPool{};
        GameObject::uMap gameObjects;
        

        // -------- -------- -------- -------- //


        public:

            // -------- CONSTRUCTOR etc -------- //

            App()
            {
                
                // Order of method chain is from top to bottom
                globalPool = 
                    DescriptorPool::Builder(ors_Device)                                         
                        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                        // .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SwapChain::MAX_FRAMES_IN_FLIGHT)
                        // .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SwapChain::MAX_FRAMES_IN_FLIGHT)
                        // .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SwapChain::MAX_FRAMES_IN_FLIGHT)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                        .build();

                loadGameObjects();

                //ui = std::make_unique<UI>(ors_Device, ors_Window, ors_Render.getSwapChainRenderPass());


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
                std::unique_ptr<Orasis::DescriptorSetLayout> globalDiscrSetLayout = 
                        DescriptorSetLayout::Builder(ors_Device)
                            // .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                            // .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                            // .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                            .build();
                
                std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

                for(int i = 0; i < globalDescriptorSets.size(); i ++)
                {
                    VkDescriptorBufferInfo bufferInfo = uniformBuffers[i]->descriptorInfo();
                    DescriptorWriter(*globalDiscrSetLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .build(globalDescriptorSets[i]);
                }

                // Systems Initialization
                // RenderSystem renderSys          {ors_Device, ors_Render.getSwapChainRenderPass(), globalDiscrSetLayout->getDescriptorSetLayout()};   
                // PointLightSystem pointLightSys  {ors_Device, ors_Render.getSwapChainRenderPass(), globalDiscrSetLayout->getDescriptorSetLayout()};   
                // ComputeSystem computeSys        {ors_Device, ors_Render.getSwapChainRenderPass(), globalDiscrSetLayout->getDescriptorSetLayout()};

                // TEST
                std::vector<VkDescriptorSetLayout> gBuffersSetLayout = {globalDiscrSetLayout->getDescriptorSetLayout(), ors_Render.getInputAttachmentSetLayout().getDescriptorSetLayout()};

                DefferedSystem defferedSys      {ors_Device, ors_Render.getSwapChainDefferedRenderPass(), globalDiscrSetLayout->getDescriptorSetLayout(),
                                                 gBuffersSetLayout, ors_Render.ors_SwapChain};
                
                Camera camera{};
                KmbMovementController cameraController{};
                GameObject cameraObj = GameObject::createGameObject();
                cameraObj.transform.translation = {0.f, -1.f, 0.f};

                auto currTime = std::chrono::high_resolution_clock::now();

                //ui->init();

                
                while(!ors_Window.shouldClose())
                {
                    glfwPollEvents();
                    
                    // Time step
                    auto newTime = std::chrono::high_resolution_clock::now();
                    float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currTime).count();
                    currTime = newTime;
                    
                    //ui->newFrame();
                    // printf("%f \n", 1/dt);
                    
                    float aspect = ors_Render.getAspectRatio();
                    cameraController.moveInPlaneXZ(ors_Window.getWindow(), cameraObj, dt);
                    camera.setViewYXZ(cameraObj.transform.translation, cameraObj.transform.rotation);
                    camera.setCameraPos(cameraObj.transform.translation);
                    camera.setPrespectiveProjection(glm::radians(60.f), aspect, 0.1f, 100.f);
                    
                    if (VkCommandBuffer cmndBuffer = ors_Render.beginFrame())
                    {
                        int frameIndex = ors_Render.getFrameIndex();

                        // Updating Frame Info
                        FrameInfo frameInfo {
                            cmndBuffer,
                            camera,
                            globalDescriptorSets[frameIndex],
                            ors_Render.getInputAttachmentDescriptorSet(frameIndex),
                            gameObjects,
                            frameIndex,
                            dt
                        };

                        if(frameIndex%2);
                            //ui->updateInfo({(dt*1000.f)});

                        // Update UBO
                        UBO_struct ubo_s{};
                        ubo_s.projection = camera.getProjection();
                        ubo_s.view = camera.getViewMatrix();
                        ubo_s.cameraPos = camera.getCameraPos();
                        
                        uniformBuffers[frameIndex]->writeToBuffer(&ubo_s);
                        uniformBuffers[frameIndex]->flush();

                        // Render
                        
                        ors_Render.startSwapChainRenderPass(cmndBuffer);

                        // renderSys.renderGameObjects(frameInfo);

                        // Deffered Render
                        defferedSys.defferedRender(frameInfo);

                        // Render ImGui
                        //ui->render(cmndBuffer);
                        
                        ors_Render.endSwapChainRenderPass(cmndBuffer);
                        ors_Render.endFrame();

                    }
                    
                }

                vkDeviceWaitIdle(ors_Device.device());

            }

        private:

            void loadGameObjects()
            {
                std::shared_ptr<Model> model = Model::createModelFromFile(ors_Device, "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/models/colored_cube.obj");
                GameObject cube = GameObject::createGameObject();
                cube.model = model;
                // cube.color = glm::vec3(1.f, 0.f, 0.f);
                cube.transform.translation = {0.f, 0.5f, 4.f};
                cube.transform.scale = glm::vec3(0.5f);
                gameObjects.emplace(cube.getID(), std::move(cube));
                
                model = Model::createModelFromFile(ors_Device, "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/models/cube.obj");
                GameObject lightCube = GameObject::createGameObject();
                lightCube.model = model;
                lightCube.transform.translation = {1.f, -3.5, -1.f};
                lightCube.transform.scale = glm::vec3(0.05f);
                gameObjects.emplace(lightCube.getID(), std::move(lightCube));
                
                model = Model::createModelFromFile(ors_Device, "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/models/quad.obj");
                GameObject quad = GameObject::createGameObject();
                quad.model = model;
                quad.transform.translation = {1.f, 1.f, -1.f};
                quad.transform.scale = glm::vec3(10);
                gameObjects.emplace(quad.getID(), std::move(quad));
                
            }



    };


}