#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"


namespace Orasis {

    class FrameBuffer {


        public:

            struct AttachmentInfo {};

            struct FrameBufferBuilder {};


        private:

            Device& m_device;
            VkFramebuffer  m_frameBuffer;

            std::unique_ptr<RenderPass> m_renderPass;
            std::vector<std::unique_ptr<Image>> m_Images;
            std::vector<VkImageView> m_imageViews;
            VkExtent2D m_extend;
        
        public:
    
            FrameBuffer(
                Device& device,
                std::unique_ptr<RenderPass> renderPass,
                std::vector<std::unique_ptr<Image>> images,
                VkExtent2D extend
            );

            ~FrameBuffer()
            {
                vkDestroyFramebuffer(m_device.device(), m_frameBuffer, nullptr);
            }
    
    };




}