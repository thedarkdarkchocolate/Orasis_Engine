#pragma once

#include "Pipeline.hpp"
#include "SwapChain.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {


    class Render {

        // -------- MEMBER VARIABLES -------- //


        Window& ors_Window;
        Device& ors_Device;
        std::unique_ptr<SwapChain> ors_SwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex {0};
        bool isFrameStarted {false};
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        Render(Window& window, Device& device)
        :ors_Window{window}, ors_Device{device}
        {
            recreateSwapChain();
            createCommandBuffers();
        }

        ~Render()
        {
            freeCommandBuffers();
        }

        Render(const Render&) = delete;
        Render &operator=(const Render&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        public:

        VkCommandBuffer beginFrame()
        {

            auto result = ors_SwapChain->acquireNextImage(&currentImageIndex);
            int currentFrameIndex;

            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapChain();
                return nullptr;
            }
            
            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                throw std::runtime_error("failed to aquire swap chain image");

            isFrameStarted = true;

            VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
            
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin command buffer");
            

            return commandBuffer;
            
        }
        
        void endFrame()
        {
            assert(isFrameStarted && "Can't call end frame while frame hasn't started");
            VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

            if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
            
            VkResult result = ors_SwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ors_Window.wasWindowResized())
            {
                ors_Window.resetWindowResizedFlag();
                recreateSwapChain();
            }
            else if (result != VK_SUCCESS)
                throw std::runtime_error("failed to present swap chain image");
                

            isFrameStarted = false;
            currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
            
        }

        void startSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {
            assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
            assert(
                commandBuffer == getCurrentCommandBuffer() &&
                "Can't begin render pass on command buffer from a different frame");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = ors_SwapChain->getRenderPass();
            renderPassInfo.framebuffer = ors_SwapChain->getFrameBuffer(currentImageIndex);
            int currentFrameIndex;
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = ors_SwapChain->getSwapChainExtent();
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(ors_SwapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(ors_SwapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, ors_SwapChain->getSwapChainExtent()};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        }
        
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {

            assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
            assert(
                commandBuffer == getCurrentCommandBuffer() &&
                "Can't end render pass on command buffer from a different frame");

            vkCmdEndRenderPass(commandBuffer);
            
        }

        bool isFrameInProgress() const
        {
            return isFrameStarted;
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }
        
        VkRenderPass getSwapChainRenderPass() const 
        {
            return ors_SwapChain->getRenderPass(); 
        }
        
        int getCurrentFrameIndex () const 
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        float getAspectRatio() const 
        {
            return ors_SwapChain->extentAspectRatio();
        }

        private:


        void createCommandBuffers()
        {

            commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

            VkCommandBufferAllocateInfo commandBufferAllocInfo{};
            commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocInfo.commandPool = ors_Device.getCommandPool();
            commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            if (vkAllocateCommandBuffers(ors_Device.device(), &commandBufferAllocInfo, commandBuffers.data()) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate command buffers");

    
        }

        void recreateSwapChain()
        {

            VkExtent2D extent = ors_Window.getExtent();

            while(extent.width == 0 || extent.height == 0)
            {
                extent = ors_Window.getExtent();
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(ors_Device.device());
            ors_SwapChain = nullptr;
            ors_SwapChain = std::make_unique<SwapChain>(ors_Device, extent);
            
            if (ors_SwapChain == nullptr) {
                ors_SwapChain = std::make_unique<SwapChain>(ors_Device, extent);
            }
            else {

                std::shared_ptr<SwapChain> oldSwapChain = std::move(ors_SwapChain);
                ors_SwapChain = std::make_unique<SwapChain>(ors_Device, extent, oldSwapChain);
                
                if (!oldSwapChain->compareSwapFormats(*ors_SwapChain.get())) 
                  throw std::runtime_error("Swap chain image(or depth) format has changed!");
                
            }
            
            
        }

      
        void freeCommandBuffers()
        {
            vkFreeCommandBuffers(ors_Device.device(), ors_Device.getCommandPool(), commandBuffers.size(), commandBuffers.data());
        }



    };


}   