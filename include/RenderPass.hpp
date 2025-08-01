#pragma once

#include "Frame_Info.hpp"
#include <vulkan/vulkan.h>


// std
#include <memory>
#include <unordered_map>

#include "vk_mem_alloc.h"


namespace Orasis {

    
    class RenderPass {

        
        public:

            struct SubpassAttachment {

                Attachment::Type            s_type{};
                VkFormat                    s_attachFormat{};
                VkAttachmentDescription     s_attchDescr{};
                uint8_t                     s_subpassToAttach{};
                uint8_t                     s_attachIndex{};
                uint8_t                     s_previousSubpass{};
            
                SubpassAttachment() = delete;

                SubpassAttachment(VkFormat attachFormat, uint8_t subpassToAttach = 0, Attachment::Type type = Attachment::Type::isColor);
                SubpassAttachment(AttachmentInfo attachment);

            };
        
            struct Builder {

                private:
                    
                    Device& s_device;
                    uint8_t attachmentNum{};
                    
                    std::unordered_map<uint8_t, std::vector<SubpassAttachment>> s_subPassAttachments{};
                    std::vector<SubpassAttachment> s_renderPassAttachmentsStruct{};
                    std::vector<VkSubpassDependency> s_dependencies{};

                public:
                    
                    Builder(Device& device)
                    : s_device(device)
                    {}

                    Builder& addSubpassAttachments(SubpassAttachment subAttachment);
                    Builder& addSubpassDependency(VkSubpassDependency dependency);

                    std::unique_ptr<RenderPass> build() const;

            };


        private:

            VkRenderPass m_renderPass;
            Device& m_device;
            

        public:
        
        // -------- CONSTRUCTOR etc -------- //
        
        RenderPass(
            Device& device,
            std::unordered_map<uint8_t, std::vector<SubpassAttachment>> subPassAttachments,
            std::vector<SubpassAttachment> renderPassAttachmentsStruct,
            std::vector<VkSubpassDependency> subpassDependancies
        );
        

        RenderPass(RenderPass& other) = delete;
        RenderPass operator=(RenderPass& other) = delete;

        ~RenderPass()
        {
            
            if (m_renderPass != VK_NULL_HANDLE) {
                vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);
                m_renderPass = VK_NULL_HANDLE;
            }

        }

        VkRenderPass renderPass()
        {
            return m_renderPass;
        }



    };


    

}