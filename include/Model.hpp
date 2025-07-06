#pragma once

#include "Device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <cstring>


namespace Orasis {

    class Model {
        
        Device& ors_Device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
        
        
        public:
        
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions()
            {
                // 1 -> binding, 2 -> stride, 3 -> inputRate
                return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
            }

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
            {
                // 1 -> location, 2 -> binding, 3 -> format, 4 -> offset
                return {    
                            //      vvv -- positions -- vvv
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)}, 

                            //      vvv -- colors -- vvv 
                            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
                        };
            }
    
        };  
        
        Model(Device& device, const std::vector<Vertex>& vertices)
        : ors_Device{device}
        {
            createVertexBuffers(vertices);
        }
        
        ~Model()
        {
            vkDestroyBuffer(ors_Device.device(), vertexBuffer, nullptr);
            vkFreeMemory(ors_Device.device(), vertexBufferMemory, nullptr);
        }
        
        
        Model(const Model&) = delete;
        Model &operator=(const Model&) = delete;



        // -------- FUNCTIONS -------- //

        void createVertexBuffers(const std::vector<Vertex>& vertices)
        {

            vertexCount = static_cast<uint32_t>(vertices.size());
            assert(vertexCount >= 3 && "Vertex count must be greater than 3");
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

            ors_Device.createBuffer(bufferSize,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    vertexBuffer,
                                    vertexBufferMemory);

            void* data;
            vkMapMemory(ors_Device.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
            vkUnmapMemory(ors_Device.device(), vertexBufferMemory);
        }
        
        void bind(VkCommandBuffer commandBuffer)
        {
            VkBuffer buffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }
        
        void draw(VkCommandBuffer commandBuffer)
        {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }

    };




}