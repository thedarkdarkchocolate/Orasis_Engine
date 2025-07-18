#pragma once

#include "Camera.hpp"

#include <vulkan/vulkan.h>

namespace Orasis {

    struct FrameInfo {

        public:

            VkCommandBuffer cmdBuffer;
            Camera camera;
            VkDescriptorSet globalDescriptorSet;
            int frameIndex;
            float dt;

            FrameInfo (VkCommandBuffer other_cmdBuffer,  Camera other_camera, VkDescriptorSet other_globalDescriptorSet, int other_frameIndex, float other_dt)
            :cmdBuffer{other_cmdBuffer},
             camera{other_camera},
             globalDescriptorSet{other_globalDescriptorSet},
             frameIndex{other_frameIndex},
             dt{other_dt}
            {}
    };

}