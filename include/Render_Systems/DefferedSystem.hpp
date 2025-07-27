#pragma once

#include "Header_Includes/Render_Systems_Headers.hpp"


namespace Orasis {

    class DefferedSystem {

        // -------- MEMBER VARIABLES -------- //

        Device& ors_Device;
        
        std::unique_ptr<Pipeline> geoPipeline;
        VkPipelineLayout geoLayout;

        std::unique_ptr<Pipeline> lightPipeline;
        VkPipelineLayout lightLayout;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        DefferedSystem(Device& device, VkRenderPass defferedRenderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout gBuffersSetLayout)
        :ors_Device{device}
        {
            createGeometryLayout(globalSetLayout);
            createGeometryPipeline(defferedRenderPass);
            
            createLightingLayout(gBuffersSetLayout);
            createLightingPipeline(defferedRenderPass);
        }

        ~DefferedSystem()
        {
            vkDestroyPipelineLayout(ors_Device.device(), geoLayout, nullptr);
            vkDestroyPipelineLayout(ors_Device.device(), lightLayout, nullptr);
        }

        DefferedSystem(const DefferedSystem&) = delete;
        DefferedSystem &operator=(const DefferedSystem&) = delete;

        // -------- -------- -------- -------- //




        // -------- FUNCTIONS -------- //

        

        void defferedRender(FrameInfo& frameInfo)
        {
            // Instansiating camera and cmdBuffer from frame info
            Camera camera = frameInfo.camera;
            VkCommandBuffer commandBuffer = frameInfo.cmdBuffer;

            geoPipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                frameInfo.cmdBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                geoLayout,
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
                    geoLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push
                );

                obj.model->bind(commandBuffer);
                obj.model->draw(commandBuffer);
            }

            vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

            lightPipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                lightLayout,
                0, 1,
                &frameInfo.globalDescriptorSet,
                0, nullptr
            );

            vkCmdDraw(commandBuffer, 6, 1, 0, 0); 

        }

        private:


        void createGeometryLayout(VkDescriptorSetLayout globalSetLayout)
        {
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            std::vector<VkDescriptorSetLayout> layout{globalSetLayout};

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layout.size());
            pipelineLayoutInfo.pSetLayouts = layout.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &geoLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   
        }

        void createLightingLayout(VkDescriptorSetLayout gBuffersSetLayout)
        {
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            std::vector<VkDescriptorSetLayout> layout{gBuffersSetLayout};
            
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layout.size());
            pipelineLayoutInfo.pSetLayouts = layout.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &lightLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");  
        }

        void createGeometryPipeline(VkRenderPass& defferedRenderPass)
        {
            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig, 3);

            pipelineConfig.renderPass = defferedRenderPass;
            pipelineConfig.pipelineLayout = geoLayout;
            pipelineConfig.subpass = 0;
            pipelineConfig.PipelineCreationFlag = 1; // 1 -> GeoPipeline
            
            geoPipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dG_shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dG_shader.frag.spv",
                pipelineConfig
            );
            
        }
        
        void createLightingPipeline(VkRenderPass& defferedRenderPass)
        {
            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig, 1);
            
            pipelineConfig.renderPass = defferedRenderPass;
            pipelineConfig.pipelineLayout = lightLayout;
            pipelineConfig.subpass = 1;
            pipelineConfig.PipelineCreationFlag = 2; // 2 -> GeoPipeline
            
            lightPipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dL_shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dL_shader.frag.spv",
                pipelineConfig
            );
            
        }


    };


}