#pragma once

#include "cjh_device.hpp"
#include "cjh_swap_chain.hpp"
#include "cjh_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace cjh
{
  class CjhRenderer
  {
  public:
    CjhRenderer(CjhWindow &window, CjhDevice &device);
    ~CjhRenderer();

    CjhRenderer(const CjhRenderer &) = delete;
    CjhRenderer &operator=(const CjhRenderer &) = delete;

    VkRenderPass getSwapChainRenderPass() const { return cjhSwapChain->getRenderPass(); }
    float getAspectRatio() const { return cjhSwapChain->extentAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandbuffer);

    VkCommandBuffer getCurrentCommandBuffer() const
    {
      assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
      return commandBuffers[currentFrameIndex];
    }

    int getFrameIndex() const
    {
      assert(isFrameStarted && "Cannot get frame index when frame not in progress");
      return currentFrameIndex;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    CjhWindow &cjhWindow;
    CjhDevice &cjhDevice;
    std::unique_ptr<CjhSwapChain> cjhSwapChain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImageIndex;
    int currentFrameIndex{0};
    bool isFrameStarted{false};
  };
} // namespace lve
