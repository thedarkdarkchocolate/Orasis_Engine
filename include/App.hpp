#pragma once

#include "Window.hpp"
#include "Pipeline.hpp"
#include "SwapChain.hpp"

#include <memory>
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
        SwapChain ors_SwapChain{ors_Device, ors_Window.getExtent()};
        std::unique_ptr<Pipeline> ors_Pipeline;

        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        App()
        {
            createPipelineLayout();
            createPipeline();
            createCommandBuffers();
        }

        ~App()
        {
            vkDestroyPipelineLayout(ors_Device.device(), pipelineLayout, nullptr);
        }

        App(const App&) = delete;
        App &operator=(const App&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        void run() {

            while(!ors_Window.shouldClose())
            {
                glfwPollEvents();
                
                drawFrame();
                
            }

            vkDeviceWaitIdle(ors_Device.device());

        }

        private:

        void createPipelineLayout() 
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            if (vkCreatePipelineLayout(ors_Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout");   


        }

        void createPipeline()
        {
            Orasis::PipelineConfigInfo pipelineConfig = Pipeline::defaultPipelineConfigInfo(ors_SwapChain.width(), ors_SwapChain.height());

            pipelineConfig.renderPass = ors_SwapChain.getRenderPass();
            pipelineConfig.pipelineLayout = pipelineLayout;

            ors_Pipeline = std::make_unique<Pipeline>
            (
                ors_Device,
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.vert.spv",
                "C:/Users/thedarkchoco/Desktop/vs_code/Orasis_Engine/shaders/compiledShaders/shader.frag.spv",
                pipelineConfig
            );


        }

        void createCommandBuffers()
        {

            commandBuffers.resize(ors_SwapChain.imageCount());

            VkCommandBufferAllocateInfo commandBufferAllocInfo{};
            commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocInfo.commandPool = ors_Device.getCommandPool();
            commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            if (vkAllocateCommandBuffers(ors_Device.device(), &commandBufferAllocInfo, commandBuffers.data()) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate command buffers");

            for(int i = 0; i < commandBuffers.size(); i++)
            {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
                    throw std::runtime_error("failed to begin command buffer");

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = ors_SwapChain.getRenderPass();
                renderPassInfo.framebuffer = ors_SwapChain.getFrameBuffer(i);

                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = ors_SwapChain.getSwapChainExtent();

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                ors_Pipeline->bind(commandBuffers[i]);

                vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

                vkCmdEndRenderPass(commandBuffers[i]);

                if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                    throw std::runtime_error("failed to record command buffer");    

            }

        }

        void drawFrame()
        {
            static uint32_t imageIndex;
            auto result = ors_SwapChain.acquireNextImage(&imageIndex);

            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                throw std::runtime_error("failed to aquire swap chain image");
                
                
                
            result = ors_SwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
            if (result != VK_SUCCESS)
                throw std::runtime_error("failed to present swap chain image");
            

            


        }


    };


}