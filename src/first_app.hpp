#pragma once

#include "vk/cjh_descriptors.hpp"
#include "vk/cjh_device.hpp"
#include "vk/cjh_game_object.hpp"
#include "vk/cjh_renderer.hpp"
#include "vk/cjh_window.hpp"

// std
#include <memory>
#include <vector>

namespace cjh
{
  class FirstApp
  {
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    FirstApp();
    ~FirstApp();

    FirstApp(const FirstApp &) = delete;
    FirstApp &operator=(const FirstApp &) = delete;

    void run();

  private:
    void loadGameObjects();

    CjhWindow cjhWindow{WIDTH, HEIGHT, "Vulkan Tutorial"};
    CjhDevice cjhDevice{cjhWindow};
    CjhRenderer cjhRenderer{cjhWindow, cjhDevice};

    // note: order of declarations matters
    std::unique_ptr<LveDescriptorPool> globalPool{};
    CjhGameObject::Map gameObjects;
  };
} // namespace lve
