#pragma once

#include "Device.hpp"
#include "Manager.hpp"

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
    
    std::unique_ptr<Manager> manager;


    // --------------------------------------
  
    // --------- Deffered Functions --------- --------- --------- --------- --------- ---------

      void initDeffered();
      void createDefferedImageViews();
      void createAttachmentImageView(VkImageView* imageView, VkImage& image, VkDeviceMemory& imageMemory, VkFormat format, VkImageUsageFlags usage);
      void createDefferedRenderPass();
      void createDefferedFramebuffers();

      public:
      
      // VkRenderPass getDefferedRenderPass() { return defferedRenderPass; }
      
      // Deffered with no manager
      // VkImageView getPositionImageViews(int index) { return positionImageViews[(index + 1 )% MAX_FRAMES_IN_FLIGHT]; }
      // VkImageView getNormalImageViews(int index) { return normalImageViews[(index + 1 )% MAX_FRAMES_IN_FLIGHT]; }
      // VkImageView getAlbedoImageViews(int index) { return albedoImageViews[(index + 1 )% MAX_FRAMES_IN_FLIGHT]; }
      
      // ------------- Manager Section -------------
      void initManager();
      
      // Deffered with manager
      VkRenderPass getDefferedRenderPass() { return manager->getRenderPass(); }
      VkImageView getPositionImageViews(int index) { return manager->getImageView("Positions", index); }
      VkImageView getNormalImageViews(int index) { return manager->getImageView("Normal", index); }
      VkImageView getAlbedoImageViews(int index) { return manager->getImageView("Albido", index); }
      
      size_t getAttachmentsCountPreSubpass(int index) { return manager->getAttachmentsCountPerSubpass(index); }
      
      VkFramebuffer getFrameBufferM(int index) { return manager->getFrameBuffer(index); }

      // size_t imageCount() { return manager->imageCount(); }
      size_t getAttachmentCount() { return manager->attachmentCount(); }

      std::vector<AttachmentInfo> getAttachments() { return manager->getAttachments(); }

      VkDescriptorSet getInputAttachmentDescriptorSet(int frameIndex) {
        return manager->getInputAttachmentDescriptorSet(frameIndex);
      }

      DescriptorSetLayout& getInputAttachmentSetLayout() {
        return manager->getInputAttachmentSetLayout();
      }
      
      

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

    size_t imageCount() { return swapChainImages.size(); } //----- If manager doesn't work need to uncomment this
    
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    VkSwapchainKHR getSwapChain() { return swapChain; }
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
