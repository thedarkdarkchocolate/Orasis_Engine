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


    class PointLightSystem {

        // -------- MEMBER VARIABLES -------- //

        Device& ors_Device;
        std::unique_ptr<Pipeline> ors_Pipeline;
        VkPipelineLayout pipelineLayout;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        :ors_Device{device}
        {

            createPipelineLayout(globalSetLayout);
            createPipeline(renderPass);
        }

        ~PointLightSystem()
        {
            vkDestroyPipelineLayout(ors_Device.device(), pipelineLayout, nullptr);
        }

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem &operator=(const PointLightSystem&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        

        void render(FrameInfo& frameInfo)
        {
            // Instansiating cmdBuffer from frame info
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

            vkCmdDraw(frameInfo.cmdBuffer, 6, 1, 0, 0);
            

        }

        private:

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
        {
            
            // VkPushConstantRange pushConstantRange{};    
            // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            // pushConstantRange.offset = 0;
            // pushConstantRange.size = sizeof(PointLightPushConstants);  

            std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   


        }

        void createPipeline(VkRenderPass renderPass)
        {

            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig);
            
            pipelineConfig.bindingDescriptions.clear();
            pipelineConfig.attributeDescriptions.clear();

            pipelineConfig.renderPass = renderPass;
            pipelineConfig.pipelineLayout = pipelineLayout;

            ors_Pipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/point_light.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/point_light.frag.spv",
                pipelineConfig
            );


        }



    };


}