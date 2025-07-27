#pragma once
 
#include "Device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace Orasis {
    
    class DescriptorSetLayout {

        public:

            class Builder {
                
                private:
                    Device& device;
                    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};

                public:

                    Builder(Device& o_device)
                    : device{o_device}
                    {}
                
                    Builder& addBinding(
                        uint32_t binding,
                        VkDescriptorType descriptorType,
                        VkShaderStageFlags stageFlags,
                        uint32_t count = 1
                    );

                    std::unique_ptr<DescriptorSetLayout> build() const;
                
            };

        private:

            Device& device;
            VkDescriptorSetLayout descriptorSetLayout;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        
        public:
        
            DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

            ~DescriptorSetLayout();

            DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
            DescriptorSetLayout& operator=(const DescriptorSetLayout& other) = delete;
            
            VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
            
        
        friend class DescriptorWriter;
    };
    

     
    class DescriptorPool {

        public:

            class Builder {
                
                private:

                    Device& device;
                    std::vector<VkDescriptorPoolSize> poolSizes{};
                    uint32_t maxSets = 1000;
                    VkDescriptorPoolCreateFlags poolFlags = 0;
                
                public:

                    Builder(Device& o_device) : device{o_device} {}
                
                    Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
                    Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
                    Builder& setMaxSets(uint32_t count);
                    std::unique_ptr<DescriptorPool> build() const;
                
            };
        
            
        private:

            Device& device;
            VkDescriptorPool descriptorPool;
        
        public:

            DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);

            ~DescriptorPool();

            DescriptorPool(const DescriptorPool& ) = delete;
            DescriptorPool& operator=(const DescriptorPool& ) = delete;
            
            bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
            
            void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
            
            void resetPool();
            
        friend class DescriptorWriter;

    };


    class DescriptorWriter {
        
        DescriptorSetLayout& setLayout;
        DescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;

        public:

            DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);
            
            DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
            DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
            
            bool build(VkDescriptorSet& set);
            void overwrite(VkDescriptorSet& set);
    };

}