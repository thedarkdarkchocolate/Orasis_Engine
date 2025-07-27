#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>


// std
#include <memory>
#include <unordered_map>

#include "vk_mem_alloc.h"


namespace Orasis {

    
    class RenderPass {

        
        public:

            struct SubpassAttachment {

                enum Type {
                    isColor,
                    isDepth,
                    isPresented
                };

                VkAttachmentDescription     s_attchDescr{};
                VkFormat                    s_attachFormat{};
                uint8_t                     s_subpassToAttach{};
                Type                        s_type{};
                uint8_t                     s_attachIndex{};
                uint8_t                     s_previousSubpass{};
            
                SubpassAttachment() = delete;

                SubpassAttachment(VkFormat attachFormat, uint8_t subpassToAttach = 0, Type type = Type::isColor);

            };
        
            struct Builder {

                private:
                    
                    Device& s_device;
                    uint8_t attachmentNum;
                    
                    std::unordered_map<uint8_t, std::vector<SubpassAttachment>> s_subPassAttachments;
                    std::vector<SubpassAttachment> s_renderPassAttachmentsStruct;
                    std::vector<VkSubpassDependency> s_dependencies;

                public:
                    
                    Builder(Device& device, const int num)
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
        )
        : m_device(device)
        {
            // The s_previous here will hold the last subpass value so it hold the total 
            uint8_t totalSubpasses = renderPassAttachmentsStruct[0].s_previousSubpass;

            std::vector<VkAttachmentDescription> renderPassAttachments = {};
            
            // Configuring Attachements Description
            for (int i = 0; i < renderPassAttachmentsStruct.size(); i++)
            {
                SubpassAttachment currAttachment = renderPassAttachmentsStruct[i];
                VkAttachmentDescription& desc = currAttachment.s_attchDescr;

                desc.format           = currAttachment.s_attachFormat;
                desc.samples          = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

                switch (currAttachment.s_type)
                {
                    case SubpassAttachment::Type::isColor:
                        desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                        desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        break;
                    case SubpassAttachment::Type::isDepth:
                        desc.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        break;
                    case SubpassAttachment::Type::isPresented:
                        desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
                        desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                        break;
                    default:
                        throw std::runtime_error("Unknown attachment type!");
                }

                renderPassAttachments.push_back(desc);
            }

            // JUST A PLACE HOLDER IN CASE IN THE FUTURE I WANT TO IMPLEMENT
            // A RENDER PASS WHERE I CHOSE IF THE PREV ATTACHMENTS NEED TO BE PASSED AS INPUT 
            bool PREV_SUBPASS_ATTACHMENTS_AS_INPUT = true; 

            std::vector<VkSubpassDescription> subPasses;
            subPasses.resize(totalSubpasses);

            std::vector<std::vector<VkAttachmentReference>> subPassesRefs;
            subPassesRefs.resize(totalSubpasses);
            
            std::vector<std::vector<VkAttachmentReference>> inputSubpassRefs;
            inputSubpassRefs.resize(totalSubpasses);
            
            VkAttachmentReference depthAttachmentRef{};

            for (int currSubpass = 0; currSubpass < totalSubpasses; currSubpass++)
            {

                bool hasDepthAttachment{false};
                bool needsPrevAttachmentsAsInput{false}; 

                for(int i = 0; i < subPassAttachments[currSubpass].size(); i++)
                {
                    auto currSubPassAttachment = subPassAttachments[currSubpass][i];

                    switch (currSubPassAttachment.s_type)
                    {
                        case SubpassAttachment::Type::isColor:
                        
                            VkAttachmentReference ref{};
                            ref.attachment = currSubPassAttachment.s_attachIndex;
                            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                            subPassesRefs[currSubpass].push_back(ref);
                            break;

                        case SubpassAttachment::Type::isDepth:

                            hasDepthAttachment = true;

                            depthAttachmentRef.attachment = currSubPassAttachment.s_attachIndex;
                            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                            break;

                        case SubpassAttachment::Type::isPresented:

                            VkAttachmentReference ref{};
                            ref.attachment = currSubPassAttachment.s_attachIndex;
                            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                            subPassesRefs[currSubpass].push_back(ref);
                            break;

                        default:
                            throw std::runtime_error("Unknown attachment type!");
                    }

                }

                if (currSubpass > 0 && PREV_SUBPASS_ATTACHMENTS_AS_INPUT)
                {
                    needsPrevAttachmentsAsInput = true;

                    for(int i = 0; i < subPassAttachments[currSubpass - 1].size(); i++)
                    {
                        auto prevSubPassAttachment = subPassAttachments[currSubpass - 1][i];

                        if(prevSubPassAttachment.s_type == SubpassAttachment::Type::isColor)
                        {
                            VkAttachmentReference ref{};
                            ref.attachment = prevSubPassAttachment.s_attachIndex;
                            ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                            inputSubpassRefs[currSubpass].push_back(ref);
                        }

                    }
                    
                }
                else 
                {
                    // Pushing back an empty ref so there are no alignment issues
                    VkAttachmentReference ref{};
                    inputSubpassRefs[currSubpass].push_back(ref);
                }

                subPasses[currSubpass] = {};
                subPasses[currSubpass].pipelineBindPoint            = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subPasses[currSubpass].colorAttachmentCount         = static_cast<uint32_t>(subPassesRefs[currSubpass].size());
                subPasses[currSubpass].pColorAttachments            = subPassesRefs[currSubpass].data();

                if (hasDepthAttachment)
                    subPasses[currSubpass].pDepthStencilAttachment  = &depthAttachmentRef;
                
                if (needsPrevAttachmentsAsInput){
                    subPasses[currSubpass].inputAttachmentCount     = static_cast<uint32_t>(inputSubpassRefs[currSubpass].size());
                    subPasses[currSubpass].pInputAttachments        = inputSubpassRefs[currSubpass].data();
                }


            }

            if (subpassDependancies.size() != totalSubpasses)
                throw std::runtime_error("dependencies count doesn't match subpass count");
            
            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount    = static_cast<uint32_t>(renderPassAttachments.size());
            renderPassInfo.pAttachments       = renderPassAttachments.data();
            renderPassInfo.subpassCount       = static_cast<uint32_t>(subPasses.size());
            renderPassInfo.pSubpasses         = subPasses.data();
            renderPassInfo.dependencyCount    = static_cast<uint32_t>(subpassDependancies.size());
            renderPassInfo.pDependencies      = subpassDependancies.data();

            if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
                throw std::runtime_error("failed to create render pass!");
            }


        }

        RenderPass(RenderPass& other) = delete;
        RenderPass operator=(RenderPass& other) = delete;

        ~RenderPass()
        {
            if (m_renderPass) 
                vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

        }

        RenderPass(const RenderPass&) = delete;
        RenderPass &operator=(const RenderPass&) = delete;


    };


    struct Image {
        // ----------------- TODO: CREATE INIT FOR DEPTH RESOURCES ----------------
        VkImage         s_image;
        VkImageView     s_imageView;
        VmaAllocation   s_allocation;   // replaces VkDeviceMemory
        VmaAllocator    s_allocator;    // passed in (not owned)

        Device&         s_device;

        Image() = delete;
        Image(const Image& o_other) = delete;
        Image operator=(const Image& o_other) = delete;
        
        Image(
            Device& device,
            VmaAllocator allocator, 
            VkExtent2D extent, 
            VkFormat format, 
            VkImageUsageFlags usage, 
            VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT)
        : s_device{device}, s_allocator{allocator}
        {
            initImage(extent, format, usage, imageAspect);
        }

        void initImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags imageAspect)
        {
            // Create Image
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            // --- VMA Allocation Info ---
            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; 

            if (vmaCreateImage(s_allocator, &imageInfo, &allocInfo,
                            &s_image, &s_allocation, nullptr) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image with VMA!");
            }

            // Create ImageView
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = s_image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = imageAspect;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(s_device.device(), &viewInfo, nullptr, &s_imageView) != VK_SUCCESS)
                throw std::runtime_error("failed to create texture image view!");
                    

        }

        ~Image()
        {
            vkDestroyImageView(s_device.device(), s_imageView, nullptr);
            vmaDestroyImage(s_allocator, s_image, s_allocation);
        }

    };

}