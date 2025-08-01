#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include "Frame_Info.hpp"
#include "Image.hpp"
#include "third_party/include/vulkan/vulkan.h"

#include <memory>

namespace Orasis {

    class FrameBuffer {


        public:

            struct Builder {

                Device&                             device;
                std::vector<AttachmentInfo>         attachments{};
                VkExtent2D                          extent{};
                uint8_t                             subpassCount = {1};
                std::vector<VkSubpassDependency>    dependancies{};

                Builder& addAttachment(AttachmentInfo attachInfo);
                Builder& addAttachment(VkFormat format, VkImageUsageFlags usage, Attachment::Type type = Attachment::Type::isColor, uint8_t subpass = 0);
                Builder& addDepthAttachment(AttachmentInfo attachInfo);
                Builder& addDepthAttachment(VkFormat format, uint8_t subpass = 0);
                Builder& addDependancy(VkSubpassDependency dependency);

            };


        private:

            Device& m_device;
            VkRenderPass m_renderPass;
            
            std::vector<VkFramebuffer> m_frameBuffers;
            VkExtent2D m_extend;
            uint8_t m_maxFrames;
        
        public:

            FrameBuffer(
                Device& device,
                VkRenderPass renderPass,
                VkExtent2D extent,
                uint8_t maxFrames
            )
            :m_device{device}, m_renderPass{renderPass}, m_extend{extent}, m_maxFrames{maxFrames}
            {
                m_frameBuffers.resize(m_maxFrames);
            }

        

            void create(std::vector<std::vector<std::shared_ptr<Image>>> images)
            {
                for (uint32_t i = 0; i < m_maxFrames; i++) {
                    std::vector<VkImageView> attachments;

                    for (auto& img : images) {
                        attachments.push_back(img[i]->s_imageView);  
                    }

                    VkFramebufferCreateInfo framebufferInfo{};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = m_renderPass;  
                    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                    framebufferInfo.pAttachments = attachments.data();
                    framebufferInfo.width = m_extend.width;
                    framebufferInfo.height = m_extend.height;
                    framebufferInfo.layers = 1;

                    if (vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create framebuffer!");
                    }
                }
            }
            
            VkFramebuffer getFrameBuffer(int currFrame)
            {
                return m_frameBuffers[currFrame];
            }


            ~FrameBuffer()
            {
                for(auto& frameBuffer: m_frameBuffers)
                    vkDestroyFramebuffer(m_device.device(), frameBuffer, nullptr);
            }
    
    };




}