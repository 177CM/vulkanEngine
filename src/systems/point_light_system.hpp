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
  class PointLightSystem
  {
  public:
    PointLightSystem(CjhDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;
    PointLightSystem &operator=(const PointLightSystem &) = delete;

    void update(FrameInfo &frameinfo, GlobalUbo &ubo);
    void render(FrameInfo &frameInfo);

  private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    CjhDevice &cjhDevice;

    std::unique_ptr<CjhPipeline> cjhPipeline;
    VkPipelineLayout m_PipelineLayout;
  };
} // namespace lve
