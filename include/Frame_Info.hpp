#pragma once

#include "Camera.hpp"
#include "GameObject.hpp"

#include "third_party/include/vulkan/vulkan.h"

namespace Orasis {


    struct UBO_struct {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        alignas(16) glm::vec3 lightPos = {1.f, -3.5f, -1.f};
        alignas(16) glm::vec3 lightColor = {1.f, 1.f, 1.f};
        alignas(16) glm::vec3 cameraPos{};
    };

    struct UI_Info {

        float dt;

    };

    struct Attachment {

        enum Type {
            isColor,
            isDepth,
            isPresented
        };
        
    };
            
    struct AttachmentInfo {

        std::string         s_name{};
        Attachment::Type    s_type{};
        VkFormat            s_format{};
        VkImageUsageFlags   s_usage{};
        uint8_t             s_subpass{};

        AttachmentInfo(std::string attachName, VkFormat format, VkImageUsageFlags usage = {}, Attachment::Type type = Attachment::Type::isColor, uint8_t subpass = 0)
        :s_name{std::move(attachName)}, s_format{format}, s_usage{usage}, s_type{type}, s_subpass{subpass}
        {}
    };


    struct ManagerInfo {
        VkFormat swapChainFormat;
        VkFormat depthFormat;
        VkSwapchainKHR swapChain;
        VkExtent2D extent;
    };

    struct FrameInfo {

        public:

            VkCommandBuffer cmdBuffer;
            Camera camera;
            VkDescriptorSet globalDescriptorSet;
            VkDescriptorSet secondaryDescriptorSet;
            GameObject::uMap& gameObjects;
            int frameIndex;
            float dt;

            FrameInfo (VkCommandBuffer o_cmdBuffer,  Camera o_camera, GameObject::uMap& o_gameObjects, int o_frameIndex, float o_dt)
            :cmdBuffer{o_cmdBuffer},
             camera{o_camera},
             gameObjects{o_gameObjects},
             frameIndex{o_frameIndex},
             dt{o_dt}
            {}
    };

}