#pragma once

#include "cjh_descriptors.hpp"
#include "cjh_device.hpp"
#include "cjh_game_object.hpp"
#include "cjh_renderer.hpp"
#include "cjh_window.hpp"

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
