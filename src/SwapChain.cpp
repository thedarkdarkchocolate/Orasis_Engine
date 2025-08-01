#include "SwapChain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace Orasis {

SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent)
    : device{deviceRef}, windowExtent{extent} {
    // init();
    // initDeffered();
    initManager();
  }
  
  SwapChain::SwapChain(Device& deviceRef, VkExtent2D extent, std::shared_ptr<SwapChain> previous)
  : device{deviceRef}, windowExtent{extent}, oldSwapChain{previous} {
    
    // init();
    // initDeffered();
    initManager();
    oldSwapChain = nullptr;
}

void SwapChain::init() {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

void SwapChain::initManager() {
  
  createSwapChain();

  swapChainDepthFormat = findDepthFormat();

  ManagerInfo managerInfo{};
  managerInfo.swapChainFormat = swapChainImageFormat;
  managerInfo.depthFormat     = swapChainDepthFormat;
  managerInfo.swapChain       = swapChain;
  managerInfo.extent          = swapChainExtent;


  manager = std::make_unique<Manager>(device, managerInfo);
  manager->createDeffered();
  
  createSyncObjects();
  
  defferedRenderPass = manager->getRenderPass();

}

// Deffered
void SwapChain::initDeffered() {
  createSwapChain();
  createDefferedImageViews();
  createDefferedRenderPass();
  createRenderPass();
  createDepthResources();
  createDefferedFramebuffers();
  createSyncObjects();
}

SwapChain::~SwapChain() {
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device.device(), imageView, nullptr);
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
    swapChain = nullptr;
  }

  for (int i = 0; i < depthImages.size(); i++) {
    vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
    vkDestroyImage(device.device(), depthImages[i], nullptr);
    vkFreeMemory(device.device(), depthImageMemorys[i], nullptr);
  }

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  
  // --------------- Deffered ---------------
  // vkDestroyRenderPass(device.device(), renderPass, nullptr);
  // for (int i = 0; i < positionImageViews.size(); i++) {
  //   vkDestroyImageView(device.device(), positionImageViews[i], nullptr);
  //   vkDestroyImage(device.device(), positionImages[i], nullptr);
  //   vkFreeMemory(device.device(), positionMemory[i], nullptr);
    
  //   vkDestroyImageView(device.device(), normalImageViews[i], nullptr);
  //   vkDestroyImage(device.device(), normalImages[i], nullptr);
  //   vkFreeMemory(device.device(), normalMemory[i], nullptr);
    
  //   vkDestroyImageView(device.device(), albedoImageViews[i], nullptr);
  //   vkDestroyImage(device.device(), albedoImages[i], nullptr);
  //   vkFreeMemory(device.device(), albedoMemory[i], nullptr);

  //   if (i == positionImageViews.size() - 1)
  //     vkDestroyRenderPass(device.device(), defferedRenderPass, nullptr);

  // }
  // --------------- Deffered ---------------

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
  }
}

VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) {
  
  vkWaitForFences(
      device.device(),
      1,
      &inFlightFences[currentFrame],
      VK_TRUE,
      std::numeric_limits<uint64_t>::max()
    );

  VkResult result = vkAcquireNextImageKHR(
      device.device(),
      swapChain,
      std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex
    );

  return result;
}

VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
  if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void SwapChain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) 
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = device.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  vkDeviceWaitIdle(device.device());
  createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

  if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  // vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
  // swapChainImages.resize(imageCount);
  // vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void SwapChain::createImageViews() {

  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }

  }

}

// Deffered Image Views
void SwapChain::createDefferedImageViews() {

  swapChainImageViews.resize(swapChainImages.size());

  positionImageViews.resize(imageCount());
  normalImageViews.resize(imageCount());
  albedoImageViews.resize(imageCount());

  positionImages.resize(imageCount());
  normalImages.resize(imageCount());
  albedoImages.resize(imageCount());
  
  positionMemory.resize(imageCount());
  normalMemory.resize(imageCount());
  albedoMemory.resize(imageCount());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;


    if (vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }


    // Hard coding format until i make an abstaction class
    createAttachmentImageView(&positionImageViews[i], positionImages[i], positionMemory[i],
       VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT); 

    createAttachmentImageView(&normalImageViews[i], normalImages[i], normalMemory[i], VK_FORMAT_R16G16B16A16_SFLOAT,
       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

    createAttachmentImageView(&albedoImageViews[i], albedoImages[i], albedoMemory[i],
       VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);



  }

}

// Deffered createAttachment
void SwapChain::createAttachmentImageView(VkImageView* imageView, VkImage& image, VkDeviceMemory& imageMemory, VkFormat format, VkImageUsageFlags usage)
{
  // Create Image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = getSwapChainExtent().width;
  imageInfo.extent.height = getSwapChainExtent().height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  
  if (vkCreateImage(device.device(), &imageInfo, nullptr, &image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image!");
  }


  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device.device(), image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = device.findMemoryType(
      memRequirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );
  
  if (vkAllocateMemory(device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate G-buffer image memory!");
  }
  
  
  // 3) Bind Memory
  if (vkBindImageMemory(device.device(), image, imageMemory, 0) != VK_SUCCESS) {
      throw std::runtime_error("failed to bind G-buffer image memory!");
  }

  // Create ImageView
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device.device(), &viewInfo, nullptr, imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }


}


void SwapChain::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  swapChainDepthFormat = depthFormat;
  VkExtent2D swapChainExtent = getSwapChainExtent();

  depthImages.resize(imageCount());
  depthImageMemorys.resize(imageCount());
  depthImageViews.resize(imageCount());

  for (int i = 0; i < depthImages.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    device.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImages[i],
        depthImageMemorys[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}


void SwapChain::createRenderPass() {

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = getSwapChainImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.dstSubpass = 0;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = 0;
  dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}


void SwapChain::createDefferedRenderPass()
{
  // 3 attachments for the G-Buffer, 1 for Depth, 1 for the SwapChain framebuffer;
  std::array<VkAttachmentDescription, 5> renderPassAttachments = {};

  // ----- G_Buffer Position - 0 ------
  renderPassAttachments[0].format           = VK_FORMAT_R16G16B16A16_SFLOAT;
  renderPassAttachments[0].samples          = VK_SAMPLE_COUNT_1_BIT;
  renderPassAttachments[0].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
  renderPassAttachments[0].storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
  renderPassAttachments[0].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  renderPassAttachments[0].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  renderPassAttachments[0].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassAttachments[0].finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  
  // ----- G_Buffer Normals - 1 ------
  renderPassAttachments[1] = renderPassAttachments[0];
  renderPassAttachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
  
  
  // ----- G_Buffer Albido - 2 ------
  renderPassAttachments[2] = renderPassAttachments[0];
  renderPassAttachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;

  // ----- Depth Attachment - 3 ------
  renderPassAttachments[3].format         = findDepthFormat();
  renderPassAttachments[3].samples        = VK_SAMPLE_COUNT_1_BIT;
  renderPassAttachments[3].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  renderPassAttachments[3].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  renderPassAttachments[3].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  renderPassAttachments[3].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassAttachments[3].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  
  // ----- Frame Buffer Color Attachment - 4 ------
  renderPassAttachments[4].format         = getSwapChainImageFormat();
  renderPassAttachments[4].samples        = VK_SAMPLE_COUNT_1_BIT;
  renderPassAttachments[4].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  renderPassAttachments[4].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  renderPassAttachments[4].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassAttachments[4].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;



  // ---- GEOMETRY SUBPASS ----- 
  
  std::array<VkAttachmentReference, 3> gBufferAttachmentRefs = {};
  for (int i = 0; i < gBufferAttachmentRefs.size(); i++)
  {
    gBufferAttachmentRefs[i].attachment = i;
    gBufferAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }
  
  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 3;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  
  VkSubpassDescription geometrySubpass = {};
  geometrySubpass.pipelineBindPoint         = VK_PIPELINE_BIND_POINT_GRAPHICS;
  geometrySubpass.colorAttachmentCount      = static_cast<uint32_t>(gBufferAttachmentRefs.size());
  geometrySubpass.pColorAttachments         = gBufferAttachmentRefs.data();
  geometrySubpass.pDepthStencilAttachment   = &depthAttachmentRef;

  // ---------------------------
  
  // ---- LIGHTING SUBPASS ----- 
  
  std::array<VkAttachmentReference, 3> lightingInputAttachmentRefs = {};
  for (int i = 0; i < lightingInputAttachmentRefs.size(); i++)
  {
    lightingInputAttachmentRefs[i].attachment = i;
    lightingInputAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }
  
  VkAttachmentReference frameBufferAtthacmentRef{};
  frameBufferAtthacmentRef.attachment = 4;
  frameBufferAtthacmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  
  VkSubpassDescription lightingSubpass = {};
  lightingSubpass.pipelineBindPoint         = VK_PIPELINE_BIND_POINT_GRAPHICS;
  lightingSubpass.inputAttachmentCount      = static_cast<uint32_t>(lightingInputAttachmentRefs.size());
  lightingSubpass.pInputAttachments         = lightingInputAttachmentRefs.data();
  lightingSubpass.colorAttachmentCount      = 1;
  lightingSubpass.pColorAttachments         = &frameBufferAtthacmentRef;  
  lightingSubpass.pDepthStencilAttachment   = nullptr;
  
  
  // ---------------------------

  std::array<VkSubpassDependency, 2> subpassDependancies = {};
  
  // External -> Geometry subpass
  subpassDependancies[0].srcSubpass       = VK_SUBPASS_EXTERNAL;
  subpassDependancies[0].srcStageMask     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependancies[0].srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT;

  // External - > dst -> Geometry Subpass index 0
  subpassDependancies[0].dstSubpass       = 0;
  subpassDependancies[0].dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependancies[0].dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependancies[0].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
  
  // Geometry subpass -> Lighting Subpass
  subpassDependancies[1].srcSubpass       = 0;
  subpassDependancies[1].srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependancies[1].srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  
  // Geometry Subpass index 0 -> Lighting Subpass index 1
  subpassDependancies[1].dstSubpass       = 1;
  subpassDependancies[1].dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  subpassDependancies[1].dstAccessMask    = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
  subpassDependancies[1].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
  
  std::array<VkSubpassDescription, 2> subPasses = {geometrySubpass, lightingSubpass};

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount    = static_cast<uint32_t>(renderPassAttachments.size());
  renderPassInfo.pAttachments       = renderPassAttachments.data();
  renderPassInfo.subpassCount       = static_cast<uint32_t>(subPasses.size());
  renderPassInfo.pSubpasses         = subPasses.data();
  renderPassInfo.dependencyCount    = static_cast<uint32_t>(subpassDependancies.size());
  renderPassInfo.pDependencies      = subpassDependancies.data();

  if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &defferedRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }

}


void SwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(imageCount());

  for (size_t i = 0; i < imageCount(); i++) {

    std::array<VkImageView, 2> attachments = {
      swapChainImageViews[i],
      depthImageViews[i]
    };

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            device.device(),
            &framebufferInfo,
            nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

// Deffered FrameBuffer
void SwapChain::createDefferedFramebuffers() {

  swapChainFramebuffers.resize(imageCount());
  
  for (size_t i = 0; i < imageCount(); i++) {

    std::array<VkImageView, 5> attachments = {
      positionImageViews[i],
      normalImageViews[i],
      albedoImageViews[i],
      depthImageViews[i],
      swapChainImageViews[i]
    };

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = defferedRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
      device.device(),
      &framebufferInfo,
      nullptr,
      &swapChainFramebuffers[i]) != VK_SUCCESS
    ) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}


void SwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {

  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {

  for (const auto &availablePresentMode : availablePresentModes) {

    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }

  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat SwapChain::findDepthFormat() {
  return device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

}