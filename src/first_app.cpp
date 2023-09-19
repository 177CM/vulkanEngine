#include "first_app.hpp"

#include "vk/keyboard_movement_controller.hpp"
#include "vk/cjh_buffer.hpp"
#include "vk/cjh_camera.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/Texture_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

#include <string>

#ifndef MODEL_PATH
#define MODEL_PATH ../
#endif

namespace cjh
{

	FirstApp::FirstApp()
	{
		globalPool =
			CjhDescriptorPool::Builder(cjhDevice)
				.setMaxSets(CjhSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CjhSwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run()
	{
		std::vector<std::unique_ptr<CjhBuffer>> uboBuffers(CjhSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<CjhBuffer>(
				cjhDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout =
			CjhDescriptorSetLayout::Builder(cjhDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(CjhSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			CjhDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{
			cjhDevice,
			cjhRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{
			cjhDevice,
			cjhRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()};
		TextureRenderSystem textureRenderSystem{
			cjhDevice,
			cjhRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout(),
			"../resources/texture/texture.jpg"};

		CjhCamera camera{};

		auto viewerObject = CjhGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		// Render Loop Here!!
		while (!cjhWindow.shouldClose())
		{
			glfwPollEvents();

			cjhUI.NewFrame();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(cjhWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = cjhRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (auto commandBuffer = cjhRenderer.beginFrame())
			{
				int frameIndex = cjhRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects};

				// update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				cjhRenderer.beginSwapChainRenderPass(commandBuffer);

				// order here matters
				simpleRenderSystem.renderGameObjects(frameInfo);
				textureRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				// ui control

				ImGui::Begin("UI Controller");
				ImGui::Text("This use for test ImGui window");
				ImGui::Checkbox("Demo Window", cjhUI.getShowDemoWindow()); // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", cjhUI.getShowAnotherWindow());
				ImGui::Checkbox("Imgui Hello World Window", cjhUI.getShowHelloWorldWindow());
				for (auto &kv : frameInfo.gameObjects)
				{
					auto &obj = kv.second;
					if (obj.model == nullptr)
						continue;
					ImGui::DragFloat3(std::to_string(obj.getId()).c_str(), (float *)&obj.transform.translation);
				}

				ImGui::End();

				// ui draw
				cjhUI.Draw();

				cjhRenderer.endSwapChainRenderPass(commandBuffer);
				cjhRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(cjhDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<CjhModel> model =
			CjhModel::createModelFromFile(cjhDevice, "models/bunny.obj");

		auto bunny = CjhGameObject::createGameObject();
		model->m_render_system = "Simple";
		bunny.model = model;
		bunny.transform.translation = glm::vec3(-.5f, .5f, 0.5f);
		bunny.transform.scale = glm::vec3(.3f) * glm::vec3(1.0f, 1.0f, 1.0f);
		bunny.transform.setIsVulkanModel(true);
		gameObjects.emplace(bunny.getId(), std::move(bunny));

		model = CjhModel::createModelFromFile(cjhDevice, "models/dragon.obj");
		model->m_render_system = "Simple";
		auto dragon = CjhGameObject::createGameObject();
		dragon.model = model;
		dragon.transform.translation = glm::vec3(.5f, .2f, 0.5f);
		dragon.transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
		dragon.transform.setIsVulkanModel(true);
		gameObjects.emplace(dragon.getId(), std::move(dragon));

		model = CjhModel::createModelFromFile(cjhDevice, "models/quad.obj");
		model->m_render_system = "Texture";
		auto floor = CjhGameObject::createGameObject();
		floor.model = model;
		floor.transform.translation = {0.f, .5f, 0.f};
		floor.transform.scale = {3.f, 1.f, 3.f};
		gameObjects.emplace(floor.getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f} //
		};

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = CjhGameObject::makePointLight(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f});
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

} // namespace lve
