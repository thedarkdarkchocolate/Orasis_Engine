#pragma once

#include "Camera.hpp"

#include <vulkan/vulkan.h>

namespace Orasis {

    struct FrameInfo {

        public:

            VkCommandBuffer cmdBuffer;
            Camera camera;
            int frameIndex;
            float dt;

            FrameInfo (VkCommandBuffer other_cmdBuffer,  Camera other_camera, int other_frameIndex, float other_dt)
            :cmdBuffer{other_cmdBuffer},
             camera{other_camera},
             frameIndex{other_frameIndex},
             dt{other_dt}
            {}
    };

}