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



    struct FrameInfo {

        public:

            VkCommandBuffer cmdBuffer;
            Camera camera;
            VkDescriptorSet globalDescriptorSet;
            GameObject::uMap& gameObjects;
            int frameIndex;
            float dt;

            FrameInfo (VkCommandBuffer o_cmdBuffer,  Camera o_camera, VkDescriptorSet o_globalDescriptorSet, GameObject::uMap& o_gameObjects, int o_frameIndex, float o_dt)
            :cmdBuffer{o_cmdBuffer},
             camera{o_camera},
             globalDescriptorSet{o_globalDescriptorSet},
             gameObjects{o_gameObjects},
             frameIndex{o_frameIndex},
             dt{o_dt}
            {}
    };

}