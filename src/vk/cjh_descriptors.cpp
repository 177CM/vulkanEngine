#pragma once
#include "cjh_descriptors.hpp"

#include <imgui.h>

// std
#include <cassert>
#include <stdexcept>

namespace cjh
{

	// *************** Descriptor Set Layout Builder *********************

	CjhDescriptorSetLayout::Builder &CjhDescriptorSetLayout::Builder::addBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags,
		uint32_t count)
	{
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<CjhDescriptorSetLayout> CjhDescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<CjhDescriptorSetLayout>(cjhDevice, bindings);
	}

	// *************** Descriptor Set Layout *********************

	CjhDescriptorSetLayout::CjhDescriptorSetLayout(
		CjhDevice &cjhDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: cjhDevice{cjhDevice}, bindings{bindings}
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (const auto kv : bindings)
		{
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(
				cjhDevice.device(),
				&descriptorSetLayoutInfo,
				nullptr,
				&descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	CjhDescriptorSetLayout::~CjhDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(cjhDevice.device(), descriptorSetLayout, nullptr);
	}

	// *************** Descriptor Pool Builder *********************

	CjhDescriptorPool::Builder &CjhDescriptorPool::Builder::addPoolSize(
		VkDescriptorType descriptorType, uint32_t count)
	{
		poolSizes.push_back({descriptorType, count});
		return *this;
	}

	CjhDescriptorPool::Builder &CjhDescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags)
	{
		poolFlags = flags;
		return *this;
	}
	CjhDescriptorPool::Builder &CjhDescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		maxSets = count;
		return *this;
	}

	std::unique_ptr<CjhDescriptorPool> CjhDescriptorPool::Builder::build() const
	{
		return std::make_unique<CjhDescriptorPool>(cjhDevice, maxSets, poolFlags, poolSizes);
	}

	// *************** Descriptor Pool *********************

	CjhDescriptorPool::CjhDescriptorPool(
		CjhDevice &cjhDevice,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize> &poolSizes)
		: cjhDevice{cjhDevice}
	{
		// Create a tight Descriptor Pool
		{
			VkDescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolInfo.pPoolSizes = poolSizes.data();
			descriptorPoolInfo.maxSets = maxSets;
			descriptorPoolInfo.flags = poolFlags;
			if (vkCreateDescriptorPool(cjhDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}
	}

	CjhDescriptorPool::~CjhDescriptorPool()
	{
		vkDestroyDescriptorPool(cjhDevice.device(), descriptorPool, nullptr);
	}

	bool CjhDescriptorPool::allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		if (vkAllocateDescriptorSets(cjhDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	void CjhDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
	{
		vkFreeDescriptorSets(
			cjhDevice.device(),
			descriptorPool,
			static_cast<uint32_t>(descriptors.size()),
			descriptors.data());
	}

	void CjhDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(cjhDevice.device(), descriptorPool, 0);
	}

	// *************** Descriptor Writer *********************

	CjhDescriptorWriter::CjhDescriptorWriter(CjhDescriptorSetLayout &setLayout, CjhDescriptorPool &pool)
		: setLayout{setLayout}, pool{pool} {}

	CjhDescriptorWriter &CjhDescriptorWriter::writeBuffer(
		uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
	{
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto &bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	CjhDescriptorWriter &CjhDescriptorWriter::writeImage(
		uint32_t binding, VkDescriptorImageInfo *imageInfo)
	{
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto &bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	bool CjhDescriptorWriter::build(VkDescriptorSet &set)
	{
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
		if (!success)
		{
			return false;
		}
		overwrite(set);
		return true;
	}

	void CjhDescriptorWriter::overwrite(VkDescriptorSet &set)
	{
		for (auto &write : writes)
		{
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool.cjhDevice.device(), writes.size(), writes.data(), 0, nullptr);
	}

} // namespace lve
