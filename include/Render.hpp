#pragma once

#include "Pipeline.hpp"
#include "SwapChain.hpp"
#include "Render_Systems/DefferedSystem.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {


    class Render {

        // -------- MEMBER VARIABLES -------- //


        Window& ors_Window;
        Device& ors_Device;
        
        std::shared_ptr<SwapChain> ors_SwapChain;

        std::vector<std::unique_ptr<Buffer>> uniformBuffers;
        std::unique_ptr<DescriptorPool> globalPool{};
        std::unique_ptr<Orasis::DescriptorSetLayout> globalDiscrSetLayout{};
        std::vector<VkDescriptorSet> globalDescriptorSets{};


        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex {0};
        bool isFrameStarted {false};
        
        // -------- -------- -------- -------- //

        public:

        std::unique_ptr<DefferedSystem> defferedSys;



        // -------- CONSTRUCTOR etc -------- //

        Render(Window& window, Device& device)
        :ors_Window{window}, ors_Device{device}
        {
            createUboDescriptors();
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
        

        void startSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {
            assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
            assert(
                commandBuffer == getCurrentCommandBuffer() &&
                "Can't begin render pass on command buffer from a different frame"
            );

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = defferedSys->def_Manager->getRenderPass();
            renderPassInfo.framebuffer = defferedSys->def_Manager->getFrameBuffer(currentImageIndex);
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = ors_SwapChain->getSwapChainExtent();

            // DEFFERED
            std::vector<VkClearValue> clearValues{};
            getClearValues(clearValues);
            

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            
            // std::array<VkClearValue, 2> clearValues{};
            // clearValues[0].color = {0.f, 0.f, 0.f, 1.f};
            // clearValues[1].depthStencil = {1.0f, 0};
            // renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            // renderPassInfo.pClearValues = clearValues.data();
            
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

        void render(FrameInfo& frameInfo)
        {
            defferedSys->defferedRender(frameInfo, {globalDescriptorSets[currentFrameIndex]});
        }
        
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {

            assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
            assert(
                commandBuffer == getCurrentCommandBuffer() &&
                "Can't end render pass on command buffer from a different frame");

            vkCmdEndRenderPass(commandBuffer);
            
        }

        void endFrame()
        {
            assert(isFrameStarted && "Can't call end frame while frame hasn't started");
            VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

            if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer");
            
            // Submit command buffer for 
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

        void createUboDescriptors()
        {
            globalPool = 
                DescriptorPool::Builder(ors_Device)                                         
                    .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();

            // Creating a total number of buffers as the number of frames in flight (each for every frame) 
            uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

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
            globalDiscrSetLayout = 
                    DescriptorSetLayout::Builder(ors_Device)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                        .build();
            
            globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

            for(int i = 0; i < globalDescriptorSets.size(); i ++)
            {
                VkDescriptorBufferInfo bufferInfo = uniformBuffers[i]->descriptorInfo();
                DescriptorWriter(*globalDiscrSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
            }
        }

        void updateBuffer(UBO_struct& ubo)
        {
            uniformBuffers[currentFrameIndex]->writeToBuffer(&ubo);
            uniformBuffers[currentFrameIndex]->flush();
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
        
        VkRenderPass getSwapChainDefferedRenderPass() const 
        {
            return ors_SwapChain->getDefferedRenderPass(); 
        }

        size_t getAttachmentCountPerSubpass(int index)
        {
            return ors_SwapChain->getAttachmentsCountPreSubpass(index);
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

        int getSwapChainImages()
        {   
            return ors_SwapChain->imageCount();
        }

        VkDescriptorSet getInputAttachmentDescriptorSet(int frameIndex) {
            return ors_SwapChain->getInputAttachmentDescriptorSet(frameIndex);
        }

        DescriptorSetLayout& getInputAttachmentSetLayout() {
            return ors_SwapChain->getInputAttachmentSetLayout();
        }

        VkImageView getPosImageView(int index) { return ors_SwapChain->getPositionImageViews(index); }
        VkImageView getNormalImageView(int index) { return ors_SwapChain->getNormalImageViews(index); }
        VkImageView getAlbidoImageView(int index) { return ors_SwapChain->getAlbedoImageViews(index); }
        

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
            
            defferedSys = std::make_unique<DefferedSystem>(ors_Device, globalDiscrSetLayout->getDescriptorSetLayout(), ors_SwapChain);
            
        }

        void getClearValues(std::vector<VkClearValue>& clearValues)
        {
            auto attachments = defferedSys->def_Manager->getAttachments();
            
            size_t size = attachments.size();

            clearValues.resize(size);

            for(int i = 0; i < size; i++)
            {
                if (attachments[i].s_type == Attachment::Type::isDepth)
                    clearValues[i] = {1.0f, 0};
                else
                    clearValues[i] = {{0.f, 0.f, 0.f, 0.f}};
            }

        }

      
        void freeCommandBuffers()
        {
            vkFreeCommandBuffers(ors_Device.device(), ors_Device.getCommandPool(), commandBuffers.size(), commandBuffers.data());
        }



    };


}   