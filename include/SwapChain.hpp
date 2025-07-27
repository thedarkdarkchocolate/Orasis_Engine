#pragma once

#include "Device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace Orasis {

  class SwapChain {

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector<VkDeviceMemory> depthImageMemorys;

    std::vector<VkImage> depthImages;
    std::vector<VkImageView> depthImageViews;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    Device& device;
    std::shared_ptr<SwapChain> oldSwapChain;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    
    // --------- Deffered Variables ---------
    VkRenderPass defferedRenderPass;

    std::vector<VkImageView> positionImageViews, normalImageViews, albedoImageViews;
    std::vector<VkImage> positionImages, normalImages, albedoImages;
    std::vector<VkDeviceMemory> positionMemory, normalMemory, albedoMemory;
    
    // --------------------------------------
  
    // --------- Deffered Functions --------- --------- --------- --------- --------- ---------

      void initDeffered();
      void createDefferedImageViews();
      void createAttachmentImageView(VkImageView* imageView, VkImage& image, VkDeviceMemory& imageMemory, VkFormat format, VkImageUsageFlags usage);
      void createDefferedRenderPass();
      void createDefferedFramebuffers();

      public:
      
      VkRenderPass getDefferedRenderPass() { return defferedRenderPass; }
      
      VkImageView getPositionImageViews(int index) { return positionImageViews[index]; }
      VkImageView getNormalImageViews(int index) { return normalImageViews[index]; }
      VkImageView getAlbedoImageViews(int index) { return albedoImageViews[index]; }
      private:

    // --------- End -> TODO: abstract the RenderPass creation and framebuffers --------- --------- --------- ---------
    
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    
    public:

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();



    SwapChain(const SwapChain &) = delete;
    SwapChain operator=(const SwapChain &) = delete;

    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkImageView getImageView(int index) { return swapChainImageViews[index]; }
    VkFormat getSwapChainImageFormat() {return swapChainImageFormat;}
    size_t imageCount() { return swapChainImages.size(); }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    uint32_t width() { return swapChainExtent.width; }
    uint32_t height() { return swapChainExtent.height; }
    VkFormat findDepthFormat();
    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    float extentAspectRatio() {
      return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }

    bool compareSwapFormats(const SwapChain &swapChain) const {
      return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
             swapChain.swapChainImageFormat == swapChainImageFormat;
    }
    
  };

}  
