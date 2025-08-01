#pragma once

#include "Device.hpp"
#include "Model.hpp"

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <assert.h>



namespace Orasis {

    // Struct for configuring pipeline
    struct PipelineConfigInfo {

        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
        uint8_t PipelineCreationFlag = 0;


    };



    class Pipeline {
        
        Device& ors_Device;

        VkPipeline graphicsPipeline;
        VkPipeline computePipeline;

        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
        VkShaderModule computeShaderModule;
        

        public:

        Pipeline(Device& ors_Device , const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo)
        : ors_Device(ors_Device)
        {
            computeShaderModule = nullptr;

            if(configInfo.PipelineCreationFlag == 0)
                createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);

            else if (configInfo.PipelineCreationFlag == 1)
                createGeometryPipeline(vertFilePath, fragFilePath, configInfo);

            else if (configInfo.PipelineCreationFlag == 2)
                createLightingPipeline(vertFilePath, fragFilePath, configInfo);

        }
        
        Pipeline(Device& ors_Device , const std::string& computeFilePath, const PipelineConfigInfo& configInfo)
        : ors_Device(ors_Device)
        {
            createComputePipeline(computeFilePath, configInfo);
        }

        Pipeline() = default;

        ~Pipeline() 
        {
            if (computeShaderModule == nullptr)
            {
                vkDestroyShaderModule(ors_Device.device(), vertShaderModule, nullptr);
                vkDestroyShaderModule(ors_Device.device(), fragShaderModule, nullptr);
                vkDestroyPipeline(ors_Device.device(), graphicsPipeline, nullptr);
            }
            else 
            {
                vkDestroyShaderModule(ors_Device.device(), computeShaderModule, nullptr);
                vkDestroyPipeline(ors_Device.device(), computePipeline, nullptr);

            }

        };



        Pipeline(const Pipeline&) = delete;
        Pipeline operator=(const Pipeline&) = delete;



        void bind(VkCommandBuffer commandBuffer)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        }


        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t colorAttachmentCount = 1)
        {

            // --- Input Assembly ---
            configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
            
            // --- Viewport ---
            configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            configInfo.viewportInfo.viewportCount = 1;
            configInfo.viewportInfo.pViewports = nullptr;
            configInfo.viewportInfo.scissorCount = 1;
            configInfo.viewportInfo.pScissors = nullptr;

            // --- Rasterization ---
            configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
            configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
            configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            configInfo.rasterizationInfo.lineWidth = 1.0f;
            configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
            configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
            configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
            // configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
            // configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
            // configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

            // --- Multisampling ---
            configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
            configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            // configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
            // configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
            // configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
            // configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
            
            // --- Color Attachments ---
            configInfo.colorBlendAttachments.resize(colorAttachmentCount);
            
            for (auto& colorBlendAttachment: configInfo.colorBlendAttachments)
            {
                colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
                // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
                // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
                // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
                // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
                // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
                // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
            }
            
            // --- Color Blending ---
            configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
            // configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
            configInfo.colorBlendInfo.attachmentCount = static_cast<uint32_t>(colorAttachmentCount);
            configInfo.colorBlendInfo.pAttachments = configInfo.colorBlendAttachments.data();
            // configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
            // configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
            // configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
            // configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
            

            configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
            configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
            configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
            // configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
            // configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
            configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
            // configInfo.depthStencilInfo.front = {};  // Optional
            // configInfo.depthStencilInfo.back = {};   // Optional

            configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
            configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
            configInfo.dynamicStateInfo.flags = 0;

            configInfo.bindingDescriptions = Model::Vertex::getBindingDescriptions();
            configInfo.attributeDescriptions = Model::Vertex::getAttributeDescriptions();

        }

    
        private:

        static std::vector<char> readFile(const std::string& filePath)
        {
            std::ifstream file{filePath, std::ios::ate | std::ios::binary};

            if(!file.is_open())
                throw std::runtime_error("failed to open file: " + filePath);

            size_t filesize = static_cast<size_t>(file.tellg());
            std::vector<char> buffer(filesize);

            file.seekg(0);
            file.read(buffer.data(), filesize);

            file.close();

            return buffer;
        }


        void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo)
        {

            assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided");


            auto vertCode = readFile(vertFilePath);
            auto fragCode = readFile(fragFilePath);

            createShaderModule(vertCode, &vertShaderModule);
            createShaderModule(fragCode, &fragShaderModule);
            
            VkPipelineShaderStageCreateInfo shaderStages[2];

            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vertShaderModule;
            shaderStages[0].pName = "main";
            shaderStages[0].flags = 0;
            shaderStages[0].pNext = nullptr;
            shaderStages[0].pSpecializationInfo = nullptr;
            
            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = fragShaderModule;
            shaderStages[1].pName = "main";
            shaderStages[1].flags = 0;
            shaderStages[1].pNext = nullptr;
            shaderStages[1].pSpecializationInfo = nullptr;
            
            auto& bindingDescriptions = configInfo.bindingDescriptions;
            auto& attributeDescriptions = configInfo.attributeDescriptions;

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
            pipelineInfo.pViewportState = &configInfo.viewportInfo;
            pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
            pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
            pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
            pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
            pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

            pipelineInfo.layout = configInfo.pipelineLayout;
            pipelineInfo.renderPass = configInfo.renderPass;
            pipelineInfo.subpass = configInfo.subpass;
            
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(ors_Device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
                throw std::runtime_error("failed to create graphics pipeline");

        }

        void createGeometryPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo)
        {
            assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided");

            auto vertCode = readFile(vertFilePath);
            auto fragCode = readFile(fragFilePath);

            createShaderModule(vertCode, &vertShaderModule);
            createShaderModule(fragCode, &fragShaderModule);
            
            VkPipelineShaderStageCreateInfo shaderStages[2];

            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vertShaderModule;
            shaderStages[0].pName = "main";
            shaderStages[0].flags = 0;
            shaderStages[0].pNext = nullptr;
            shaderStages[0].pSpecializationInfo = nullptr;
            
            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = fragShaderModule;
            shaderStages[1].pName = "main";
            shaderStages[1].flags = 0;
            shaderStages[1].pNext = nullptr;
            shaderStages[1].pSpecializationInfo = nullptr;
            
            auto& bindingDescriptions = configInfo.bindingDescriptions;
            auto& attributeDescriptions = configInfo.attributeDescriptions;

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
            pipelineInfo.pViewportState = &configInfo.viewportInfo;
            pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
            pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
            pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
            pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
            pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

            pipelineInfo.layout = configInfo.pipelineLayout;
            pipelineInfo.renderPass = configInfo.renderPass;
            pipelineInfo.subpass = configInfo.subpass;
            
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(ors_Device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
                throw std::runtime_error("failed to create graphics pipeline");

        }
        
        void createLightingPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo)
        {
            assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided");


            auto vertCode = readFile(vertFilePath);
            auto fragCode = readFile(fragFilePath);

            createShaderModule(vertCode, &vertShaderModule);
            createShaderModule(fragCode, &fragShaderModule);
            
            VkPipelineShaderStageCreateInfo shaderStages[2];

            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vertShaderModule;
            shaderStages[0].pName = "main";
            shaderStages[0].flags = 0;
            shaderStages[0].pNext = nullptr;
            shaderStages[0].pSpecializationInfo = nullptr;
            
            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = fragShaderModule;
            shaderStages[1].pName = "main";
            shaderStages[1].flags = 0;
            shaderStages[1].pNext = nullptr;
            shaderStages[1].pSpecializationInfo = nullptr;
            

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexBindingDescriptions = nullptr;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr;


            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
            pipelineInfo.pViewportState = &configInfo.viewportInfo;
            pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
            pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
            pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
            pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
            pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

            pipelineInfo.layout = configInfo.pipelineLayout;
            pipelineInfo.renderPass = configInfo.renderPass;
            pipelineInfo.subpass = configInfo.subpass;
            
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(ors_Device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
                throw std::runtime_error("failed to create graphics pipeline");

        }

        void createComputePipeline(const std::string& computeFilePath, const PipelineConfigInfo& configInfo)
        {
            assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided");

            auto computeCode = readFile(computeFilePath);

            createShaderModule(computeCode, &computeShaderModule);

            VkPipelineShaderStageCreateInfo computeStage{};
            computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeStage.module = computeShaderModule;
            computeStage.pName = "main";
            computeStage.flags = 0;
            computeStage.pNext = nullptr;
            computeStage.pSpecializationInfo = nullptr;

            VkComputePipelineCreateInfo computeCreateInfo{};
            computeCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            computeCreateInfo.stage = computeStage;
            computeCreateInfo.layout = configInfo.pipelineLayout;

            if (vkCreateComputePipelines(ors_Device.device(), VK_NULL_HANDLE, 1, &computeCreateInfo, nullptr, &computePipeline) != VK_SUCCESS)
                throw std::runtime_error("failed to create compute pipeline");
        }   

        void createShaderModule(const std::vector<char>& code,  VkShaderModule* shaderModule)
        {
            VkShaderModuleCreateInfo createInfo{};
            
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            if (vkCreateShaderModule(ors_Device.device(), &createInfo, nullptr, shaderModule))
                throw std::runtime_error("failed to create shader module");
        }
    };


}