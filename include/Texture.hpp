#pragma once 

#include "Device.hpp"
#include "Frame_Info.hpp"
#include "Image.hpp"


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <string>
#include <memory>

#include "vk_mem_alloc.h"

namespace Orasis {


    class Texture {


        Device& m_device;
        VmaAllocator m_allocator;
        std::shared_ptr<Image> m_image;
        VkSampler m_sampler{};

        public:
            
            Texture(Device& device, VmaAllocator allocator, const std::string& filepath)
            : m_device{device}, m_allocator{allocator}
            {
                int texWidth, texHeight, texChannels;
                std::vector<unsigned char> pixels;
                loadFromFile(filepath, texWidth, texHeight, pixels);



                uint32_t indexSize = sizeof(pixels[0]);
                VkDeviceSize bufferSize = sizeof(pixels[0]) * pixels.size();

                Buffer stagingBuffer(
                    m_device,
                    indexSize,
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_MEMORY_USAGE_AUTO_PREFER_HOST
                );

                stagingBuffer.map();
                stagingBuffer.writeToBuffer(pixels.data());
                stagingBuffer.unmap();

                VkExtent2D texExtent {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)};

                // Creating attachment here should have another constructor to input AttachmentInfo direclty
                AttachmentInfo attachInfo {"", VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, Attachment::Type::isTexture};

                m_image = Image::createAttachment(
                    m_device,
                    m_allocator,
                    texExtent,
                    attachInfo
                );

                m_device.copyBufferToImage(stagingBuffer.getBuffer(), m_image->s_image, texWidth, texHeight, 1);
                
                createSampler();

            }


            ~Texture()
            {
                vkDestroySampler(m_device.device(), m_sampler, nullptr);
            }

        private:

            void loadFromFile(const std::string& filepath, int& texWidth, int& texHeight, std::vector<unsigned char>& pixels)
            {
                int texChannels;
                stbi_uc* data = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                if (!data) {
                    throw std::runtime_error("failed to load texture file: " + filepath);
                }

                pixels.assign(data, data + texWidth * texHeight * 4);
                stbi_image_free(data);
            }

            void createSampler()
            {
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.anisotropyEnable = VK_TRUE;

                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(m_device.physicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

                if(vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
                    throw std::runtime_error("failed to create texture sampler");


            }



    };





}


