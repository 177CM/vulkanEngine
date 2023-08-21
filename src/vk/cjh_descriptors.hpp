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

		friend class CjhDescriptorWriter;
	};

	class CjhDescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(CjhDevice &cjhDevice) : cjhDevice{cjhDevice} {}

			Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder &setMaxSets(uint32_t count);
			std::unique_ptr<CjhDescriptorPool> build() const;

		private:
			CjhDevice &cjhDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		CjhDescriptorPool(
			CjhDevice &cjhDevice,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize> &poolSizes);
		~CjhDescriptorPool();
		CjhDescriptorPool(const CjhDescriptorPool &) = delete;
		CjhDescriptorPool &operator=(const CjhDescriptorPool &) = delete;

		VkDescriptorPool get() { return descriptorPool; }

		bool
		allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

		void resetPool();

	private:
		CjhDevice &cjhDevice;
		VkDescriptorPool descriptorPool;

		friend class CjhDescriptorWriter;
	};

	class CjhDescriptorWriter
	{
	public:
		CjhDescriptorWriter(CjhDescriptorSetLayout &setLayout, CjhDescriptorPool &pool);

		CjhDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
		CjhDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

		bool build(VkDescriptorSet &set);
		void overwrite(VkDescriptorSet &set);

	private:
		CjhDescriptorSetLayout &setLayout;
		CjhDescriptorPool &pool;
		std::vector<VkWriteDescriptorSet> writes;
	};

} // namespace lve
