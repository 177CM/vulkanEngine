#pragma once

#include "vk/cjh_camera.hpp"
#include "vk/cjh_device.hpp"
#include "vk/cjh_frame_info.hpp"
#include "vk/cjh_game_object.hpp"
#include "vk/cjh_pipeline.hpp"

#include "vk/cjh_image.hpp"
#include "vk/cjh_descriptors.hpp"
#include "vk/cjh_swap_chain.hpp"

// std
#include <memory>
#include <vector>

namespace cjh
{
  class TextureRenderSystem
  {
  public:
    TextureRenderSystem(CjhDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string paths);
    ~TextureRenderSystem();

    TextureRenderSystem(const TextureRenderSystem &) = delete;
    TextureRenderSystem &operator=(const TextureRenderSystem &) = delete;

    void renderGameObjects(FrameInfo &frameInfo);

  private:
    void createImages(std::vector<std::string> path);
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);
    CjhImage m_cjhImage;
    std::vector<CjhImage> m_cjhImages;
    CjhDevice &cjhDevice;

    std::unique_ptr<CjhPipeline> cjhPipeline;
    std::vector<VkDescriptorSet> imageSampleSets;
    std::unique_ptr<CjhDescriptorPool> m_textureRenderSystemPool;
    VkPipelineLayout m_PipelineLayout;
  };
} // namespace lve
