#include "cjh_renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace cjh
{

  CjhRenderer::CjhRenderer(CjhWindow &window, CjhDevice &device)
      : cjhWindow{window}, cjhDevice{device}
  {
    recreateSwapChain();
    createCommandBuffers();
  }

  CjhRenderer::~CjhRenderer() { freeCommandBuffers(); }

  void CjhRenderer::recreateSwapChain()
  {
    auto extent = cjhWindow.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
      extent = cjhWindow.getExtent();
      glfwWaitEvents();
    }
    vkDeviceWaitIdle(cjhDevice.device());

    if (cjhSwapChain == nullptr)
    {
      cjhSwapChain = std::make_unique<CjhSwapChain>(cjhDevice, extent);
    }
    else
    {
      std::shared_ptr<CjhSwapChain> oldSwapChain = std::move(cjhSwapChain);
      cjhSwapChain = std::make_unique<CjhSwapChain>(cjhDevice, extent, oldSwapChain);

      if (!oldSwapChain->compareSwapFormats(*cjhSwapChain.get()))
      {
        throw std::runtime_error("Swap chain image(or depth) format has changed!");
      }
    }
  }

  void CjhRenderer::createCommandBuffers()
  {
    commandBuffers.resize(CjhSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cjhDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(cjhDevice.device(), &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }

  void CjhRenderer::freeCommandBuffers()
  {
    vkFreeCommandBuffers(
        cjhDevice.device(),
        cjhDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
  }

  VkCommandBuffer CjhRenderer::beginFrame()
  {
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = cjhSwapChain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      recreateSwapChain();
      return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
  }

  void CjhRenderer::endFrame()
  {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to record command buffer!");
    }

    auto result = cjhSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        cjhWindow.wasWindowResized())
    {
      cjhWindow.resetWindowResizedFlag();
      recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % CjhSwapChain::MAX_FRAMES_IN_FLIGHT;
  }

  void CjhRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = cjhSwapChain->getRenderPass();
    renderPassInfo.framebuffer = cjhSwapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = cjhSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(cjhSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(cjhSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, cjhSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void CjhRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
  }

  VkCommandBuffer CjhRenderer::beginSingleTimeCommands()
  {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cjhDevice.getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(cjhDevice.device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }
  void CjhRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(cjhDevice.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(cjhDevice.graphicsQueue());

    vkFreeCommandBuffers(cjhDevice.device(), cjhDevice.getCommandPool(), 1, &commandBuffer);
  }

} // namespace lve
