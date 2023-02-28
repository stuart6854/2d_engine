#include "texture.hpp"

#include "device.hpp"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace app::gfx
{
    struct Texture::TexturePimpl
    {
        Device* device = nullptr;

        u32 width = 0;
        u32 height = 0;
        vk::Format format{};

        vk::Image image{};
        VmaAllocation allocation{};
        vk::ImageView view{};

        vk::DescriptorSet set{};
    };

    Texture::Texture(Device* device) : m_pimpl(new TexturePimpl)
    {
        m_pimpl->device = device;
    }

    Texture::~Texture()
    {
        destroy();
    }

    void Texture::init(const std::string& filename)
    {
        destroy();

        auto device = m_pimpl->device->get_device();
        auto allocator = m_pimpl->device->get_allocator();

        int w, h, c;
        byte* data = stbi_load(filename.c_str(), &w, &h, &c, 4);

        m_pimpl->width = w;
        m_pimpl->height = h;
        m_pimpl->format = vk::Format::eR8G8B8A8Srgb;

        vk::ImageCreateInfo image_info{};
        image_info.imageType = vk::ImageType::e2D;
        image_info.format = m_pimpl->format;
        image_info.extent = vk::Extent3D{ m_pimpl->width, m_pimpl->height, 1 };
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

        VkImageCreateInfo vk_image_info = image_info;
        VkImage vk_image;
        vmaCreateImage(allocator, &vk_image_info, &alloc_info, &vk_image, &m_pimpl->allocation, nullptr);
        m_pimpl->image = vk_image;

        sizet data_size = static_cast<sizet>(w * h * 4);
        m_pimpl->device->upload_to_image(m_pimpl->image, image_info.extent, data_size, data);

        vk::ImageViewCreateInfo view_info{};
        view_info.viewType = vk::ImageViewType::e2D;
        view_info.image = m_pimpl->image;
        view_info.format = m_pimpl->format;
        view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        m_pimpl->view = device.createImageView(view_info);
    }

    void Texture::destroy()
    {
        if (!m_pimpl->image)
        {
            return;
        }

        auto device = m_pimpl->device->get_device();
        auto allocator = m_pimpl->device->get_allocator();

        device.destroy(m_pimpl->view);
        device.freeDescriptorSets(m_pimpl->device->get_descriptor_pool(), m_pimpl->set);

        vmaDestroyImage(allocator, m_pimpl->image, m_pimpl->allocation);

        m_pimpl->image = nullptr;
        m_pimpl->allocation = nullptr;
        m_pimpl->view = nullptr;
        m_pimpl->width = 0;
        m_pimpl->height = 0;
    }

}
