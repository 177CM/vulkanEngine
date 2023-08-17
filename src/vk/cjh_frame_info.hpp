#pragma once

#include "cjh_camera.hpp"
#include "cjh_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace cjh
{

#define MAX_LIGHTS 10

	struct PointLight
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{};	  // w is color intensity
	};

	struct GlobalUbo
	{
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::mat4 inverseView{1.0f};
		glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.02f};
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		CjhCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		CjhGameObject::Map &gameObjects;
	};
} // namespace lve
