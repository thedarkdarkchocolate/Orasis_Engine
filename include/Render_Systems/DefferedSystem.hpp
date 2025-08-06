#pragma once

#include "Header_Includes/Render_Systems_Headers.hpp"

namespace Orasis {

    struct SimplePushConstantData 
    {
        glm::mat4 modelMatrix{1.f};
    };


    class DefferedSystem {

        // -------- MEMBER VARIABLES -------- //

        Device& m_device;
        std::unique_ptr<Manager> def_Manager;
        std::shared_ptr<SwapChain> m_swapChain;

        std::unique_ptr<Pipeline> geoPipeline;
        VkPipelineLayout geoLayout;

        std::unique_ptr<Pipeline> lightPipeline;
        VkPipelineLayout lightLayout;

        VkDescriptorSetLayout globalSetLayout;

        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        DefferedSystem(Device& device, VkDescriptorSetLayout globalSetLayout, std::shared_ptr<SwapChain> swapChain)
        :m_device{device}, globalSetLayout{globalSetLayout}, m_swapChain{swapChain}
        {

            ManagerInfo mngrInfo;
            mngrInfo.swapChain =        swapChain->getSwapChain();
            mngrInfo.swapChainFormat =  swapChain->getSwapChainImageFormat();
            mngrInfo.depthFormat =      swapChain->findDepthFormat();
            mngrInfo.extent =           swapChain->getSwapChainExtent();

            def_Manager = std::make_unique<Manager>(device, mngrInfo);
            
            createGeometryLayout({globalSetLayout});
            createGeometryPipeline(def_Manager->getRenderPass());
            
            createLightingLayout({globalSetLayout, def_Manager->getInputAttachmentSetLayout().getDescriptorSetLayout()});
            createLightingPipeline(def_Manager->getRenderPass());


        }

        ~DefferedSystem()
        {
            vkDestroyPipelineLayout(m_device.device(), geoLayout, nullptr);
            vkDestroyPipelineLayout(m_device.device(), lightLayout, nullptr);
        }

        DefferedSystem(const DefferedSystem&) = delete;
        DefferedSystem &operator=(const DefferedSystem&) = delete;

        // -------- -------- -------- -------- //




        // -------- FUNCTIONS -------- //

        

        void defferedRender(FrameInfo& frameInfo, std::vector<VkDescriptorSet> descriptors)
        {
            // Instansiating camera and cmdBuffer from frame info
            Camera camera = frameInfo.camera;
            VkCommandBuffer commandBuffer = frameInfo.cmdBuffer;

            geoPipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                frameInfo.cmdBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                geoLayout,
                0,
                static_cast<uint32_t>(descriptors.size()),
                descriptors.data(),
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

            descriptors.push_back(def_Manager->getInputAttachmentDescriptorSet(frameInfo.frameIndex));
            
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                lightLayout,
                0,
                static_cast<uint32_t>(descriptors.size()),
                descriptors.data(),
                0, nullptr
            );

            vkCmdDraw(commandBuffer, 6, 1, 0, 0); 

        }

        private:


        void createGeometryLayout(std::vector<VkDescriptorSetLayout> layoutToSet)
        {
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layoutToSet.size());
            pipelineLayoutInfo.pSetLayouts = layoutToSet.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &geoLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   
        }

        void createLightingLayout(std::vector<VkDescriptorSetLayout> layoutToSet)
        {
            VkPushConstantRange pushConstantRange{};    
            pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(SimplePushConstantData);  

            
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layoutToSet.size());
            pipelineLayoutInfo.pSetLayouts = layoutToSet.data();
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &lightLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");  
        }

        void createGeometryPipeline(VkRenderPass defferedRenderPass)
        {
            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo(pipelineConfig, def_Manager->getAttachmentsCountPerSubpass(0) - 1);

            pipelineConfig.renderPass = defferedRenderPass;
            pipelineConfig.pipelineLayout = geoLayout;
            pipelineConfig.subpass = 0;
            pipelineConfig.PipelineCreationFlag = 1; // 1 -> GeoPipeline
            
            geoPipeline = std::make_unique<Pipeline>
            (
                m_device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dG_shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dG_shader.frag.spv",
                pipelineConfig
            );
            
        }
        
        void createLightingPipeline(VkRenderPass defferedRenderPass)
        {
            Orasis::PipelineConfigInfo pipelineConfig{};
            Pipeline::defaultPipelineConfigInfo (pipelineConfig, def_Manager->getAttachmentsCountPerSubpass(1));
            
            pipelineConfig.renderPass = defferedRenderPass;
            pipelineConfig.pipelineLayout = lightLayout;
            pipelineConfig.subpass = 1;
            pipelineConfig.PipelineCreationFlag = 2; // 2 -> GeoPipeline
            
            lightPipeline = std::make_unique<Pipeline>
            (
                m_device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dL_shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/dL_shader.frag.spv",
                pipelineConfig
            );
            
        }

        public:
        friend class Render;
    };


}