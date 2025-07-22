#pragma once 

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


#include <vulkan/vulkan.h>

namespace Orasis {


    class UI {

        Device& m_device;
        Window& m_window;
        VkCommandBuffer m_cmdBuffer;
        VkRenderPass m_renderPass;
        VkDescriptorPool imguiPool = VK_NULL_HANDLE;

        UI_Info m_info{};



        public:

            UI(Device& device, Window& window, VkRenderPass renderPass)
            : m_device{device}, m_window{window}, m_renderPass{renderPass}
            {}

            ~UI()
            {
                cleanup();
            }

            void init()
            {

                createDescriptorPool();

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                // io.ConfigFlags  |= ImGuiConfigFlags_NavEnableKeyboard 
                //                 | ImGuiConfigFlags_NavEnableGamepad 
                //                 | ImGuiConfigFlags_DockingEnable 
                //                 | ImGuiConfigFlags_ViewportsEnable;
                
                ImGui_ImplGlfw_InitForVulkan(m_window.getWindow(), true);

                QueueFamilyIndices queueFamilyIndices = m_device.findPhysicalQueueFamilies();
                
                ImGui_ImplVulkan_InitInfo init_info = {};
                init_info.Instance = m_device.instance();
                init_info.PhysicalDevice = m_device.physicalDevice();
                init_info.Device = m_device.device();
                init_info.QueueFamily = queueFamilyIndices.graphicsFamily;
                init_info.Queue = m_device.graphicsQueue();
                init_info.PipelineCache = VK_NULL_HANDLE;
                init_info.DescriptorPool = imguiPool;
                init_info.Subpass = 0;
                init_info.MinImageCount = 2;
                init_info.ImageCount = 2;
                init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
                init_info.Allocator = nullptr;
                init_info.CheckVkResultFn = checkVkResult;
                init_info.RenderPass = m_renderPass;
                // init_info.UseDynamicRendering = true;

                if (!ImGui_ImplVulkan_Init(&init_info))
                    throw std::runtime_error("failed ImGui_ImplVulkan_Init");


                
                // Set up the dark theme
                ImGuiStyle& style = ImGui::GetStyle();
                style.WindowMinSize        = ImVec2(160, 20);
                style.FramePadding         = ImVec2(4, 2);
                style.ItemSpacing          = ImVec2(6, 2);
                style.ItemInnerSpacing     = ImVec2(6, 4);
                style.Alpha                = 1.0f;
                style.WindowRounding       = 0.0f;
                style.FrameRounding        = 0.0f;
                style.IndentSpacing        = 6.0f;
                style.ColumnsMinSpacing    = 50.0f;
                style.GrabMinSize          = 14.0f;
                style.GrabRounding         = 0.0f;
                style.ScrollbarSize        = 12.0f;
                style.ScrollbarRounding    = 0.0f;

                // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                //     style.WindowRounding = 0.0f;
                //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
                // }
                

            }

            void render(VkCommandBuffer commandBuffer) {
                
                ImGui::Begin("Kappa");
                ImGui::SetWindowPos(ImVec2(-1, 1));
                ImGui::Text("%2f ms ", m_info.dt);
                ImGui::End();

                ImGui::Render();
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            }

            void newFrame() {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

            }

            void updateInfo(UI_Info info)
            {
                m_info = std::move(info);
            }


        private:
            
            void createDescriptorPool()
            {
                VkDescriptorPoolSize pool_sizes[] = { 
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
                };

                VkDescriptorPoolCreateInfo pool_info = {};
                pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                pool_info.maxSets = 1000;
                pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
                pool_info.pPoolSizes = pool_sizes;

                if(vkCreateDescriptorPool(m_device.device(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
                    throw std::runtime_error("failed to create imgui descriptor pool");

            }
            
            static void checkVkResult(VkResult err) {
                if (err == 0) return;
                fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
                if (err < 0) abort();
            }

            void cleanup() {
                if (imguiPool != VK_NULL_HANDLE) {
                    ImGui_ImplVulkan_Shutdown();
                    ImGui_ImplGlfw_Shutdown();
                    ImGui::DestroyContext();
                    
                    vkDestroyDescriptorPool(m_device.device(), imguiPool, nullptr);
                    imguiPool = VK_NULL_HANDLE;
                }
            }

    };



}