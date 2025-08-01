#pragma once

#include "Frame_Info.hpp"
#include <vulkan/vulkan.h>


// std
#include <memory>
#include <unordered_map>

// #include "vk_mem_alloc.h"

namespace Orasis {



    struct Image {
    
        VkImage         s_image;
        VkImageView     s_imageView;
        VmaAllocation   s_allocation; 
        VmaAllocator    s_allocator;  
        Device&         s_device;
        
    
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
            createAttachment(extent, format, usage, imageAspect);
        }

        Image(Device& device)
        : s_device{device}
        {}

        void createAttachment(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags imageAspect)
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

        // static std::shared_ptr<Image> createDepthImage(
        //     Device& device,
        //     VmaAllocator allocator,
        //     VkExtent2D extent,
        //     VkFormat depthFormat
        // ) {
        //     return std::make_shared<Image>(
        //         device,
        //         allocator,
        //         extent,
        //         depthFormat,
        //         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        //         VK_IMAGE_ASPECT_DEPTH_BIT
        //     );
        // }

        // static std::shared_ptr<Image> createColorAttachment(
        //     Device& device,
        //     VmaAllocator allocator,
        //     VkExtent2D extent,
        //     VkFormat colorFormat,
        //     VkImageUsageFlags usage
        // ) {
        //     return std::make_shared<Image>(
        //         device,
        //         allocator,
        //         extent,
        //         colorFormat,
        //         usage,
        //         VK_IMAGE_ASPECT_COLOR_BIT
        //     );
        // }

        static std::shared_ptr<Image> createAttachment(
            Device& device,
            VmaAllocator allocator,
            VkExtent2D extent,
            AttachmentInfo attachment
        ) {
            return std::make_shared<Image>(
                device,
                allocator,
                extent,
                attachment.s_format,
                attachment.s_usage,
                attachment.s_type == Attachment::Type::isDepth ?
                    VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT
            );
        }

        static std::shared_ptr<Image> wrapSwapchainImage(
            Device& device,
            VkImage image,
            VkFormat format
        ) {
            auto img = std::make_shared<Image>(device);
            img->s_image = image;
            img->s_allocator = nullptr;  // not owned
            img->s_allocation = nullptr; // not owned

            // Create ImageView
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &img->s_imageView) != VK_SUCCESS)
                throw std::runtime_error("failed to create image view for swapchain image!");

            return img;
        }


        ~Image()
        {
            vkDestroyImageView(s_device.device(), s_imageView, nullptr);
            if (s_allocator && s_allocation) {
                vmaDestroyImage(s_allocator, s_image, s_allocation);
            }
        }



    };

}