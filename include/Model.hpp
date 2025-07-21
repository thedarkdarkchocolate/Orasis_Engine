#pragma once

#include "Device.hpp"
#include "Buffer.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstring>




namespace Orasis {


    class Model {

        
        public:
        
            struct Vertex
            {
                glm::vec3 position{};
                glm::vec3 color{};
                glm::vec3 normal{};
                glm::vec2 uv{};
                
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
                                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},

                                //      vvv -- normals -- vvv 
                                {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},

                                //      vvv -- uvs -- vvv 
                                {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)}
                            };
                }

                bool operator==(const Vertex& other) const
                {
                    return (position == other.position && color == other.color && normal == other.normal && uv == other.uv);
                }
        
            };  

            struct Builder {

                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};

                void loadModel(const std::string& filepath);      

            };

        private:
                        
            Device& ors_Device;
            bool hasIndexBuffer {false};

            // Vertex Buffer, memory, count
            std::unique_ptr<Buffer> vertexBuffer;
            uint32_t vertexCount;
            
            // Index Buffer, memory, count
            std::unique_ptr<Buffer> indexBuffer;
            uint32_t indexCount;
        
        public:    
            
            
            Model(Device& device, const Builder& builder)
            : ors_Device{device}
            {
                createVertexBuffers(builder.vertices);
                createIndexBuffers(builder.indices);
            }
            
            ~Model() {}
            
            
            Model(const Model&) = delete;
            Model &operator=(const Model&) = delete;



            // -------- FUNCTIONS -------- //

            void createVertexBuffers(const std::vector<Vertex>& vertices)
            {
                
                vertexCount = static_cast<uint32_t>(vertices.size());
                assert(vertexCount >= 3 && "Vertex count must be greater than 3");
                
                uint32_t vertexSize = sizeof(vertices[0]);
                VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
                
                Buffer stagingBuffer {
                    ors_Device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                };
                
                stagingBuffer.map();
                stagingBuffer.writeToBuffer((void *)vertices.data());
                
                
                vertexBuffer = std::make_unique<Buffer>(
                    ors_Device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );

                // Copying from staging to local device buffer
                ors_Device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

            }
                
            void createIndexBuffers(const std::vector<uint32_t>& indices)
            {
                indexCount = static_cast<uint32_t>(indices.size());
                hasIndexBuffer = indexCount > 0;

                if (!hasIndexBuffer) return; 
                
                uint32_t indexSize = sizeof(indices[0]);
                VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

                Buffer stagingBuffer {
                    ors_Device,
                    indexSize,
                    indexCount,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                };
                
                stagingBuffer.map();
                stagingBuffer.writeToBuffer((void *)indices.data());
                
                
                indexBuffer = std::make_unique<Buffer>(
                    ors_Device,
                    indexSize,
                    indexCount,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );
                

                // Copying from staging to local device buffer
                ors_Device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

            }

            static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath)
            {
                Builder builder;
                builder.loadModel(filepath);

                return std::make_unique<Model>(device, builder);
            }


            void bind(VkCommandBuffer commandBuffer)
            {
                VkBuffer buffers[] = {vertexBuffer->getBuffer()};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                
                if (hasIndexBuffer)
                    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

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

