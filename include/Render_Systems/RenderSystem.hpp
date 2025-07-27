#pragma once

#include "Header_Includes/Render_Systems_Headers.hpp"


namespace Orasis {

    struct SimplePushConstantData 
    {
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

        RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        :ors_Device{device}
        {
            
            createPipelineLayout(globalSetLayout);
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

        

        void renderGameObjects(FrameInfo& frameInfo)
        {
            // Instansiating camera and cmdBuffer from frame info
            Camera camera = frameInfo.camera;
            VkCommandBuffer commandBuffer = frameInfo.cmdBuffer;

            ors_Pipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                frameInfo.cmdBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0, 1,
                &frameInfo.globalDescriptorSet,
                0, nullptr
            );
            

            for (auto& kv: frameInfo.gameObjects)
            {
                GameObject& obj = kv.second;

                if (obj.model == nullptr) continue;

                SimplePushConstantData push{};

                glm::mat4 modelMatrix = obj.transform.mat4();
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

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
        {

            
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
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
            pipelineConfig.PipelineCreationFlag = 0;

            ors_Pipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.frag.spv",
                pipelineConfig
            );


        }

    };


}