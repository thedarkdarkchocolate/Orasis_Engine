#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Camera.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "GameObject.hpp"
#include "Frame_Info.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {

    struct SimplePushConstantData 
    {
        glm::mat4 transform{1.f};
        glm::mat4 modelMatrix{1.f};
    };

    class RenderSystem {

        // -------- MEMBER VARIABLES -------- //

        Device& ors_Device;
        std::unique_ptr<Pipeline> ors_Pipeline;
        VkPipelineLayout pipelineLayout;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        RenderSystem(Device& device, VkRenderPass renderPass)
        :ors_Device{device}
        {

            createPipelineLayout();
            createPipeline(renderPass);
        }

        ~RenderSystem()
        {
            vkDestroyPipelineLayout(ors_Device.device(), pipelineLayout, nullptr);
        }

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem &operator=(const RenderSystem&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        

        void renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects)
        {
            // Instansiating camera and cmdBuffer from frame info
            Camera camera = frameInfo.camera;
            VkCommandBuffer commandBuffer = frameInfo.cmdBuffer;

            ors_Pipeline->bind(commandBuffer);

            
            glm::mat4 projectionView = camera.getProjection() * camera.getViewMatrix();

            for (GameObject& obj: gameObjects)
            {

                SimplePushConstantData push{};

                glm::mat4 modelMatrix = obj.transform.mat4();
                push.transform = projectionView * modelMatrix;
                push.modelMatrix = modelMatrix;

                vkCmdPushConstants (
                    commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push
                );

                obj.model->bind(commandBuffer);
                obj.model->draw(commandBuffer);
            }


        }

        private:

        void createPipelineLayout() 
        {

            
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   


        }

        void createPipeline(VkRenderPass renderPass)
        {

            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig);

            pipelineConfig.renderPass = renderPass;
            pipelineConfig.pipelineLayout = pipelineLayout;

            ors_Pipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.frag.spv",
                pipelineConfig
            );


        }


       

        // void verticesForTringleInATringle(int depth, glm::mat3x2 startVert, std::vector<Model::Vertex>& out)
        // {
            
        //     if (depth-- <= 0) return;
            
        //     // for(int i = 0; i < 3; i++)
        //     // {
        //     //     glm::vec2 top = startVert[0];
        //     //     glm::vec2 left = (startVert[0] + startVert[1]) / 2.f;
        //     //     glm::vec2 right = (startVert[0] + startVert[2]) / 2.f;
                
                
        //     //     out.push_back({{top.x, top.y}});
        //     //     out.push_back({{left.x, left.y}});
        //     //     out.push_back({{right.x, right.y}});
                
        //     //     verticesForTringleInATringle(depth, {top, left, right}, out);

        //     // }

        //     glm::vec2 top = startVert[0];
        //     glm::vec2 left = (startVert[0] + startVert[1]) / 2.f;
        //     glm::vec2 right = (startVert[0] + startVert[2]) / 2.f;
            
        //     if (!depth){
        //     out.push_back({{top.x, top.y}});
        //     out.push_back({{left.x, left.y}});
        //     out.push_back({{right.x, right.y}});
        //     }
        //     verticesForTringleInATringle(depth, {top, left, right}, out);

        //     top = (startVert[0] + startVert[1]) / 2.f;;
        //     left = startVert[1];
        //     right = (startVert[1] + startVert[2]) / 2.f;

        //     if (!depth){
        //     out.push_back({{top.x, top.y}});
        //     out.push_back({{left.x, left.y}});
        //     out.push_back({{right.x, right.y}});
        //     }
        //     verticesForTringleInATringle(depth, {top, left, right}, out);

        //     top = (startVert[0] + startVert[2]) / 2.f;
        //     left = (startVert[1] + startVert[2]) / 2.f;
        //     right = (startVert[2]);

        //     if (!depth){
        //     out.push_back({{top.x, top.y}});
        //     out.push_back({{left.x, left.y}});
        //     out.push_back({{right.x, right.y}});
        //      }   
        //     verticesForTringleInATringle(depth, {top, left, right}, out);


        // }



    };


}