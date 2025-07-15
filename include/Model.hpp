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
        bool hasIndexBuffer {false};

        // Vertex Buffer, memory, count
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
        
        // Index Buffer, memory, count
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t indexCount;
        
        
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

            struct Builder {

                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};

            };

            
            
            Model(Device& device, const Builder& builder)
            : ors_Device{device}
            {
                createVertexBuffers(builder.vertices);
                createIndexBuffers(builder.indices);
            }
            
            ~Model()
            {
                // Vertex Buffer cleanup
                vkDestroyBuffer(ors_Device.device(), vertexBuffer, nullptr);
                vkFreeMemory(ors_Device.device(), vertexBufferMemory, nullptr);
                
                // Index Buffer cleanup
                if (hasIndexBuffer)
                {
                    vkDestroyBuffer(ors_Device.device(), indexBuffer, nullptr);
                    vkFreeMemory(ors_Device.device(), indexBufferMemory, nullptr);
                }
            }
            
            
            Model(const Model&) = delete;
            Model &operator=(const Model&) = delete;



            // -------- FUNCTIONS -------- //

            void createVertexBuffers(const std::vector<Vertex>& vertices)
            {
                
                vertexCount = static_cast<uint32_t>(vertices.size());
                assert(vertexCount >= 3 && "Vertex count must be greater than 3");
                
                VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
                
                // Staging buffer
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;

                // Creating Staging Buffer
                ors_Device.createBuffer
                (
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory
                );
                
                // Copying vertices to staging buffer
                void* data;
                vkMapMemory(ors_Device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
                vkUnmapMemory(ors_Device.device(), stagingBufferMemory);
                
                // Creating local device buffer
                ors_Device.createBuffer
                (
                    bufferSize,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    vertexBuffer,
                    vertexBufferMemory
                );

                // Copying from staging to local device buffer
                ors_Device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

                // Destroying staging buffer after vertices got copied to local device buffer
                vkDestroyBuffer(ors_Device.device(), stagingBuffer, nullptr);
                vkFreeMemory(ors_Device.device(), stagingBufferMemory, nullptr);
                    
            }
                
            void createIndexBuffers(const std::vector<uint32_t>& indices)
            {
                indexCount = static_cast<uint32_t>(indices.size());
                hasIndexBuffer = indexCount > 0;

                if (!hasIndexBuffer) return; 
                
                VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
                
                // Staging buffer
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;

                // Creating Staging Buffer
                ors_Device.createBuffer
                (
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory
                );
                
                // Copying indices to staging buffer
                void* data;
                vkMapMemory(ors_Device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
                vkUnmapMemory(ors_Device.device(), stagingBufferMemory);
                
                // Creating local device buffer
                ors_Device.createBuffer
                (
                    bufferSize,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    indexBuffer,
                    indexBufferMemory
                );

                // Copying from staging to local device buffer
                ors_Device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

                // Destroying staging buffer after indices got copied to local device buffer
                vkDestroyBuffer(ors_Device.device(), stagingBuffer, nullptr);
                vkFreeMemory(ors_Device.device(), stagingBufferMemory, nullptr);
            }


            void bind(VkCommandBuffer commandBuffer)
            {
                VkBuffer buffers[] = {vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                
                if (hasIndexBuffer)
                    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            }
            

            void draw(VkCommandBuffer commandBuffer)
            {
                if (hasIndexBuffer)
                    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
                else
                    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
            }
                
    };
            
            
            

}