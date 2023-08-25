#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>
#include "stb_image.h"
#include "cjh_device.hpp"
#include "cjh_buffer.hpp"

namespace cjh
{
    class CjhImage
    {
    public:
        CjhImage(CjhDevice &cjhDevice, std::string Path);
        ~CjhImage();
        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        // helper functions
        VkImage image() const { return m_image; }
        VkImageView imageView() const { return m_imageView; }

    private:
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_imageSampler;
        CjhDevice &m_cjhDevice;
        int m_imgWidth;
        int m_imgHeight;
        int m_channels;

        void createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void creatImageView(VkFormat format);
        void createImageSampler();
    };
}