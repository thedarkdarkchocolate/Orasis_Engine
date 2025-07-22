#pragma once

#include "Header_Includes/Render_Systems_Headers.hpp"


namespace Orasis {

    class ComputeSystem {

        // -------- MEMBER VARIABLES -------- //

        Device& ors_Device;
        std::unique_ptr<Pipeline> ors_Pipeline;
        VkPipelineLayout pipelineLayout;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        ComputeSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        :ors_Device{device}
        {

            createPipelineLayout(globalSetLayout);
            createPipeline(renderPass);
        }

        ~ComputeSystem()
        {
            vkDestroyPipelineLayout(ors_Device.device(), pipelineLayout, nullptr);
        }

        ComputeSystem(const ComputeSystem&) = delete;
        ComputeSystem &operator=(const ComputeSystem&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        

        void dispatchCompute(FrameInfo& frameInfo, glm::vec3 groupCount)
        {
            VkCommandBuffer commandBuffer = frameInfo.cmdBuffer;

            ors_Pipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0, 1,
                &frameInfo.globalDescriptorSet,
                0, nullptr
            );

            vkCmdDispatch(commandBuffer, groupCount.x, groupCount.y, groupCount.z);
        }

        private:

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
        {
            
            std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   


        }

        void createPipeline(VkRenderPass renderPass)
        {

            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig);

            pipelineConfig.pipelineLayout = pipelineLayout;

            ors_Pipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/compute.comp.spv",
                pipelineConfig
            );


        }

    };


}