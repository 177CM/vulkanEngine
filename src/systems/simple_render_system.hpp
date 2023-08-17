#pragma once

#include "vk/cjh_camera.hpp"
#include "vk/cjh_device.hpp"
#include "vk/cjh_frame_info.hpp"
#include "vk/cjh_game_object.hpp"
#include "vk/cjh_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace cjh
{
  class SimpleRenderSystem
  {
  public:
    SimpleRenderSystem(CjhDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

    void renderGameObjects(FrameInfo &frameInfo);

  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    CjhDevice &cjhDevice;

    std::unique_ptr<CjhPipeline> cjhPipeline;
    VkPipelineLayout m_PipelineLayout;
  };
} // namespace lve
