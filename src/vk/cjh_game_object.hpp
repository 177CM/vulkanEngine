#pragma once

#include "cjh_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace cjh
{

  struct TransformComponent
  {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();
    glm::mat3 normalMatrix();
    inline void setIsVulkanModel(bool flag)
    {
      isVulkanModel = flag;
    }

  private:
    bool isVulkanModel = false;
  };

  struct PointLightComponent
  {
    float lightIntensity = 1.0f;
  };

  class CjhGameObject
  {
  public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, CjhGameObject>;

    static CjhGameObject createGameObject()
    {
      static id_t currentId = 0;
      return CjhGameObject{currentId++};
    }

    static CjhGameObject makePointLight(
        float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    CjhGameObject(const CjhGameObject &) = delete;
    CjhGameObject &operator=(const CjhGameObject &) = delete;
    CjhGameObject(CjhGameObject &&) = default;
    CjhGameObject &operator=(CjhGameObject &&) = default;

    id_t getId() { return id; }

    glm::vec3 color{};
    TransformComponent transform{};

    // Optional pointer components
    std::shared_ptr<CjhModel> model{};
    std::unique_ptr<PointLightComponent> pointLight = nullptr;

  private:
    CjhGameObject(id_t objId) : id{objId} {}

    id_t id;
  };
} // namespace lve
