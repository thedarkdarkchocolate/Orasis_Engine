#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include "Descriptors.hpp"
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


            //TODO static map_manager["SSAO"]

            Device& m_device;
            VkSwapchainKHR m_swapChain;

            std::unique_ptr<RenderPass> m_renderPass{};
            std::unique_ptr<FrameBuffer> m_frameBuffer{};

            std::unique_ptr<DescriptorPool> m_managerPool{};
            std::unique_ptr<Orasis::DescriptorSetLayout> m_managerDiscrSetLayout{};
            std::vector<VkDescriptorSet> m_managerDescriptorSets{};

            VmaAllocator m_allocator;
            VkExtent2D m_extent;
            VkFormat m_swapChainImageFormat;
            VkFormat m_depthFormat;

            std::unordered_map<std::string ,std::vector<std::shared_ptr<Image>>> m_imagesMap;
            std::vector<std::vector<std::shared_ptr<Image>>> m_imagesArray;

            std::vector<AttachmentInfo> m_attachments;
            std::vector<std::vector<AttachmentInfo>> m_attachmentsPerSubpass;


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

                createDescriptors();
            }
            
            void createDeffered()
            {
                int maxFrameInFlight = 3;

                // Set deffered Attachments
                std::array<AttachmentInfo, 5> attachments = {
                    AttachmentInfo("Positions", VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Normal", VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Albido", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
                    AttachmentInfo("Depth", m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, Attachment::Type::isDepth),
                    AttachmentInfo("OutColor", m_swapChainImageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, Attachment::Type::isPresented, 1)
                };

                int totalSubpasses = attachments.back().s_subpass + 1;
                m_attachmentsPerSubpass.resize(totalSubpasses);

                // Transfering attachment to member variable
                for (int i = 0; i < attachments.size(); i++)
                {
                    m_attachments.push_back(attachments[i]);
                    m_attachmentsPerSubpass[attachments[i].s_subpass].push_back(attachments[i]);
                }
                m_attachments.shrink_to_fit();
                
                m_imagesArray.resize(attachments.size());

                // create Images
                for(int attachIndex = 0; attachIndex < attachments.size(); attachIndex++){

                    AttachmentInfo currAttachment = attachments[attachIndex];

                    if(currAttachment.s_type == Attachment::Type::isPresented)
                        createSwapChainImages(currAttachment, attachIndex);

                    else{

                        for (int i = 0; i < maxFrameInFlight; i++)
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
                {
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
                    
                }
                
                
                for (int i = 0; i < subpassDependancies.size(); i++)
                    builder.addSubpassDependency(subpassDependancies[i]);
                
                // create RenderPass
                m_renderPass = builder.build();
                
                
                // create FrameBuffer 
                m_frameBuffer = std::make_unique<FrameBuffer>(m_device, m_renderPass->renderPass(), m_extent, m_imageCount);
                m_frameBuffer->create(m_imagesArray);


            }


            void createDescriptors()
            {
                // ------------------- Descriptors ------------------- 
                
                m_managerPool = DescriptorPool::Builder(m_device)
                .setMaxSets(2)
                .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3 * m_imageCount) 
                .build();
                
                m_managerDiscrSetLayout = DescriptorSetLayout::Builder(m_device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
                
                m_managerDescriptorSets.resize(m_imageCount);

                for (int i = 0; i < m_imageCount; ++i) {
                    VkDescriptorImageInfo posInfo{};
                    posInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    posInfo.imageView = m_imagesMap["Positions"][0]->s_imageView;
                    posInfo.sampler = VK_NULL_HANDLE;

                    VkDescriptorImageInfo normInfo{};
                    normInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    normInfo.imageView = m_imagesMap["Normal"][0]->s_imageView;
                    normInfo.sampler = VK_NULL_HANDLE;

                    VkDescriptorImageInfo albInfo{};
                    albInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    albInfo.imageView = m_imagesMap["Albido"][0]->s_imageView;
                    albInfo.sampler = VK_NULL_HANDLE;

                    DescriptorWriter(*m_managerDiscrSetLayout, *m_managerPool)
                        .writeImage(0, &posInfo)
                        .writeImage(1, &normInfo)
                        .writeImage(2, &albInfo)
                        .build(m_managerDescriptorSets[i]);
                }


                
                // ------------------- End Descriptors ------------------- 
            }
            
            VkImage getImage(std::string name, int index)                           { return m_imagesMap[name][index]->s_image; }
            VkImageView getImageView(std::string name, int index)                   { return m_imagesMap[name][index]->s_imageView; }
            VkRenderPass getRenderPass()                                            { return m_renderPass->renderPass(); }
            VkFramebuffer getFrameBuffer(int frameIndex)                            { return m_frameBuffer->getFrameBuffer(frameIndex); }
            size_t imageCount()                                                     { return m_imageCount; }
            size_t attachmentCount()                                                { return m_imagesArray.size(); }
            std::vector<AttachmentInfo> getAttachments()                            { return m_attachments; }
            std::vector<std::vector<AttachmentInfo>> getAttachmentsPerSubpass()     { return m_attachmentsPerSubpass; }
            size_t getAttachmentsCountPerSubpass(int frameIndex)                    { return m_attachmentsPerSubpass[frameIndex].size(); }
            VkDescriptorSet getInputAttachmentDescriptorSet(int frameIndex)         { return m_managerDescriptorSets[frameIndex]; }
            DescriptorSetLayout& getInputAttachmentSetLayout()                      { return *m_managerDiscrSetLayout; }

            void createSwapChainImages(AttachmentInfo attachment, uint32_t attachIndex)
            {
                std::vector<VkImage> swapchainImages;

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