#pragma once

#include "cjh_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace cjh
{

	class CjhDescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder(CjhDevice &cjhDevice) : cjhDevice{cjhDevice} {}
			Builder &addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<CjhDescriptorSetLayout> build() const;

		private:
			CjhDevice &cjhDevice;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		CjhDescriptorSetLayout(
			CjhDevice &cjhDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~CjhDescriptorSetLayout();
		CjhDescriptorSetLayout(const CjhDescriptorSetLayout &) = delete;
		CjhDescriptorSetLayout &operator=(const CjhDescriptorSetLayout &) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		CjhDevice &cjhDevice;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class LveDescriptorWriter;
	};

	class LveDescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(CjhDevice &cjhDevice) : cjhDevice{cjhDevice} {}

			Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder &setMaxSets(uint32_t count);
			std::unique_ptr<LveDescriptorPool> build() const;

		private:
			CjhDevice &cjhDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		LveDescriptorPool(
			CjhDevice &cjhDevice,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize> &poolSizes);
		~LveDescriptorPool();
		LveDescriptorPool(const LveDescriptorPool &) = delete;
		LveDescriptorPool &operator=(const LveDescriptorPool &) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

		void resetPool();

	private:
		CjhDevice &cjhDevice;
		VkDescriptorPool descriptorPool;

		friend class LveDescriptorWriter;
	};

	class LveDescriptorWriter
	{
	public:
		LveDescriptorWriter(CjhDescriptorSetLayout &setLayout, LveDescriptorPool &pool);

		LveDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
		LveDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

		bool build(VkDescriptorSet &set);
		void overwrite(VkDescriptorSet &set);

	private:
		CjhDescriptorSetLayout &setLayout;
		LveDescriptorPool &pool;
		std::vector<VkWriteDescriptorSet> writes;
	};

} // namespace lve
