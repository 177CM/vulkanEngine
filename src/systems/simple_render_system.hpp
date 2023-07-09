#pragma once

#include "cjh_camera.hpp"
#include "cjh_device.hpp"
#include "cjh_frame_info.hpp"
#include "cjh_game_object.hpp"
#include "cjh_pipeline.hpp"

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

    std::unique_ptr<CjhPipeline> lvePipeline;
    VkPipelineLayout pipelineLayout;
  };
} // namespace lve
