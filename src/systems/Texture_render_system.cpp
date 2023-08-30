#include "texture_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace cjh
{

  struct TexturePushConstantData
  {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
  };

  TextureRenderSystem::TextureRenderSystem(CjhDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
      : cjhDevice{device}, m_cjhImage(device, "../resources/texture/texture.jpg")
  {

    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
  }

  TextureRenderSystem::~TextureRenderSystem()
  {
    vkDestroyPipelineLayout(cjhDevice.device(), m_PipelineLayout, nullptr);
  }

  void TextureRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
  {

    // create a pool for image sampler
    m_textureRenderSystemPool =
        CjhDescriptorPool::Builder(cjhDevice)
            .setMaxSets(CjhSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, CjhSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    auto imageSampleSetLayout =
        CjhDescriptorSetLayout::Builder(cjhDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

    imageSampleSets.resize(CjhSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < imageSampleSets.size(); i++)
    {
      auto imageInfo = m_cjhImage.descriptorInfo();
      CjhDescriptorWriter(*imageSampleSetLayout, *m_textureRenderSystemPool)
          .writeImage(0, &imageInfo)
          .build(imageSampleSets[i]);
    }

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(TexturePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout,
                                                            imageSampleSetLayout->getDescriptorSetLayout()};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(cjhDevice.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) !=
        VK_SUCCESS)
    {
      throw std::runtime_error("failed to create pipeline layout!");
    }
  }

  void TextureRenderSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CjhPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    cjhPipeline = std::make_unique<CjhPipeline>(
        cjhDevice,
        "shaders/spv/texture_shader.vert.spv",
        "shaders/spv/texture_shader.frag.spv",
        pipelineConfig);
  }

  void TextureRenderSystem::renderGameObjects(
      FrameInfo &frameInfo)
  {
    cjhPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_PipelineLayout,
                            0,
                            1,
                            &frameInfo.globalDescriptorSet,
                            0,
                            nullptr);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_PipelineLayout,
                            1,
                            1,
                            &imageSampleSets[frameInfo.frameIndex],
                            0,
                            nullptr);

    for (auto &kv : frameInfo.gameObjects)
    {
      auto &obj = kv.second;
      if (obj.model == nullptr)
        continue;
      TexturePushConstantData push{};
      push.modelMatrix = obj.transform.mat4();
      push.normalMatrix = obj.transform.normalMatrix();

      vkCmdPushConstants(
          frameInfo.commandBuffer,
          m_PipelineLayout,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          0,
          sizeof(TexturePushConstantData),
          &push);
      obj.model->bind(frameInfo.commandBuffer);
      obj.model->draw(frameInfo.commandBuffer);
    }
  }

} // namespace lve
