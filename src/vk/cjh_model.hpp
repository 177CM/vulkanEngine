#pragma once

#include "cjh_buffer.hpp"
#include "cjh_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

#include <chrono>
#include <iostream>

namespace cjh
{
  class CjhModel
  {
  public:
    struct Vertex
    {
      glm::vec3 position{};
      glm::vec3 color{};
      glm::vec3 normal{};
      glm::vec2 uv{};

      static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

      bool operator==(const Vertex &other) const
      {
        return position == other.position && color == other.color && normal == other.normal &&
               uv == other.uv;
      }
    };

    struct Timer
    {
      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      std::chrono::duration<float> duration;
      Timer()
      {
        start = std::chrono::high_resolution_clock::now();
      }
      ~Timer()
      {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        float ms = duration.count() * 1000.0f;
        std::cout << "Timer took " << ms << "ms" << std::endl;
      }
    };

    struct Builder
    {
      std::vector<Vertex> vertices{};
      std::vector<uint32_t> indices{};

      void loadModel(const std::string &filepath);
    };

    CjhModel(CjhDevice &device, const CjhModel::Builder &builder);
    ~CjhModel();

    CjhModel(const CjhModel &) = delete;
    CjhModel &operator=(const CjhModel &) = delete;

    static std::unique_ptr<CjhModel> createModelFromFile(
        CjhDevice &device, const std::string &filepath);

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

  private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    CjhDevice &cjhDevice;

    std::unique_ptr<CjhBuffer> vertexBuffer;
    uint32_t vertexCount;

    bool hasIndexBuffer = false;
    std::unique_ptr<CjhBuffer> indexBuffer;
    uint32_t indexCount;
  };
} // namespace lve
