#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

#include "FrameBuffer.hpp"
#include "Image.hpp"
#include "Frame_Info.hpp"

#include <string>
#include <array>
#include <unordered_map>

namespace Orasis {


    class Manager {

        private:

            Device& m_device;
            VkSwapchainKHR m_swapChain;

            std::unique_ptr<RenderPass> m_renderPass;
            std::unique_ptr<FrameBuffer> m_frameBuffer;

            VmaAllocator m_allocator;
            VkExtent2D m_extent;
            VkFormat m_swapChainImageFormat;
            VkFormat m_depthFormat;

            std::unordered_map<std::string ,std::vector<std::shared_ptr<Image>>> m_imagesMap;
            std::vector<std::vector<std::shared_ptr<Image>>> m_imagesArray;

            uint32_t m_imageCount;
            
        public:

            Manager(Device& device, ManagerInfo managerInfo)
            :m_device{device},
             m_swapChain{managerInfo.swapChain},
             m_swapChainImageFormat{managerInfo.swapChainFormat},
             m_depthFormat{managerInfo.depthFormat},
             m_extent{managerInfo.extent}
            {
                // Gets Vulkan lowest image count that it supports and choose the preffered imageCount
                aquireImageCount();
                
                initilizesAllocator();
                createDeffered();
            }
            
            void createDeffered()
            {
                // Set deffered Attachments
                std::array<AttachmentInfo, 5> attachments = {
                    AttachmentInfo("Positions", VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Normal", VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Albido", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Depth", m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, Attachment::Type::isDepth),
                    AttachmentInfo("OutColor", m_swapChainImageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, Attachment::Type::isPresented, 1)
                };
                
                
                m_imagesArray.resize(attachments.size());

                // create Images
                for(int attachIndex = 0; attachIndex < attachments.size(); attachIndex++){

                    AttachmentInfo currAttachment = attachments[attachIndex];

                    if(currAttachment.s_type == Attachment::Type::isPresented)
                        createSwapChainImages(currAttachment, attachIndex);
                        
                    else{
                        for (int i = 0; i < m_imageCount; i++)
                        {
                            m_imagesMap[currAttachment.s_name].push_back(Image::createAttachment(m_device, m_allocator, m_extent, currAttachment));
                        }

                        m_imagesArray[attachIndex] = m_imagesMap[currAttachment.s_name];
                    }
                }
                
                RenderPass::Builder builder (m_device);
                for(int i = 0; i < attachments.size(); i++)
                    builder.addSubpassAttachments(RenderPass::SubpassAttachment(attachments[i]));
                
                std::array<VkSubpassDependency, 2> subpassDependancies = {};
                // External -> Geometry subpass
                subpassDependancies[0].srcSubpass       = VK_SUBPASS_EXTERNAL;
                subpassDependancies[0].srcStageMask     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                subpassDependancies[0].srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT;
                
                // External - > dst -> Geometry Subpass index 0
                subpassDependancies[0].dstSubpass       = 0;
                subpassDependancies[0].dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependancies[0].dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                subpassDependancies[0].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
                
                // Geometry subpass -> Lighting Subpass
                subpassDependancies[1].srcSubpass       = 0;
                subpassDependancies[1].srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependancies[1].srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                
                // Geometry Subpass index 0 -> Lighting Subpass index 1
                subpassDependancies[1].dstSubpass       = 1;
                subpassDependancies[1].dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                subpassDependancies[1].dstAccessMask    = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                subpassDependancies[1].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
                
                
                for (int i = 0; i < subpassDependancies.size(); i++)
                    builder.addSubpassDependency(subpassDependancies[i]);
                
                // create RenderPass
                m_renderPass = builder.build();
                
                
                // create FrameBuffer 
                m_frameBuffer = std::make_unique<FrameBuffer>(m_device, m_renderPass->renderPass(), m_extent, m_imageCount);
                m_frameBuffer->create(m_imagesArray);
            }
            
            VkImage getImage(std::string name, int index)
            {
                return m_imagesMap[name][index]->s_image;
            }
            
            VkImageView getImageView(std::string name, int index)
            {
                return m_imagesMap[name][index]->s_imageView;
            }

            VkRenderPass getRenderPass()
            {
                return m_renderPass->renderPass();
            }

            VkFramebuffer getFrameBuffer(int index)
            {
                return m_frameBuffer->getFrameBuffer(index);
            }

            size_t imageCount()
            {
                return m_imageCount;
            }

            void createSwapChainImages(AttachmentInfo attachment, uint32_t attachIndex)
            {
                std::vector<VkImage> swapchainImages;

                // we only specified a minimum number of images in the swap chain, so the implementation is
                // allowed to create a swap chain with more. That's why we'll first query the final number of
                // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
                // retrieve the handles
                vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &m_imageCount, nullptr);
                swapchainImages.resize(m_imageCount);
                vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &m_imageCount, swapchainImages.data());

                for (auto& image : swapchainImages)
                    m_imagesMap[attachment.s_name].push_back(Image::wrapSwapchainImage(m_device, image, attachment.s_format));
                

                m_imagesArray[attachIndex] = m_imagesMap[attachment.s_name];

            }

            void aquireImageCount()
            {

                SwapChainSupportDetails swapChainSupport = m_device.getSwapChainSupport();

                m_imageCount = swapChainSupport.capabilities.minImageCount + 1;

                if (swapChainSupport.capabilities.maxImageCount > 0 &&
                    m_imageCount > swapChainSupport.capabilities.maxImageCount) 
                {
                    m_imageCount = swapChainSupport.capabilities.maxImageCount;
                }
            }
            
            ~Manager() {

                m_imagesMap.clear();
                m_imagesArray.clear();

                if (m_allocator != nullptr) {
                    vmaDestroyAllocator(m_allocator);
                }
            }            
            
            private: 

            void initilizesAllocator()
            {
                VmaVulkanFunctions vulkanFunctions{};
                vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
                vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
                vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
                vulkanFunctions.vkFreeMemory = vkFreeMemory;
                vulkanFunctions.vkMapMemory = vkMapMemory;
                vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
                vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
                vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
                vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
                vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
                vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
                vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
                vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
                vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
                vulkanFunctions.vkCreateImage = vkCreateImage;
                vulkanFunctions.vkDestroyImage = vkDestroyImage;
                vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

                
                vulkanFunctions.vkGetBufferMemoryRequirements2KHR =
                    (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(m_device.device(), "vkGetBufferMemoryRequirements2KHR");
                vulkanFunctions.vkGetImageMemoryRequirements2KHR =
                    (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(m_device.device(), "vkGetImageMemoryRequirements2KHR");


                VmaAllocatorCreateInfo allocatorInfo{};
                allocatorInfo.physicalDevice = m_device.physicalDevice();  
                allocatorInfo.device = m_device.device();                  
                allocatorInfo.instance = m_device.instance();              
                // allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
                // allocatorInfo.pVulkanFunctions = &vulkanFunctions;

                if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create VMA allocator");
                }
            }

            
        };
        
        
        

}