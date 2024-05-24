#include "../include/image.hpp"

void eng::image::create_image(device &dev, image_create_options &opts) {
  m_device = dev.get_device();

  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = opts.width;
  image_info.extent.height = opts.height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = opts.format;
  image_info.tiling = opts.tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = opts.usage;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(m_device, &image_info, nullptr, opts.image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(m_device, *(opts.image), &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex =
      dev.find_memory_type(mem_requirements.memoryTypeBits, opts.properties);

  if (vkAllocateMemory(m_device, &alloc_info, nullptr, opts.image_memory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(m_device, *(opts.image), *(opts.image_memory), 0);
}

void eng::texture::create_texture(device &dev, const char *file_path) {
  m_device = dev.get_device();

  int width, height, channels;

  stbi_uc *pixels =
      stbi_load(file_path, &width, &height, &channels, STBI_rgb_alpha);
  VkDeviceSize image_size = width * height * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;

  buffer_create_options buffer_opts{};
  buffer_opts.size = image_size;
  buffer_opts.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  buffer_opts.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  buffer_opts.buffer = &staging_buffer;
  buffer_opts.buffer_memory = &staging_buffer_memory;

  dev.create_buffer(buffer_opts);

  void *data;
  vkMapMemory(m_device, staging_buffer_memory, 0, image_size, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(image_size));
  vkUnmapMemory(m_device, staging_buffer_memory);

  stbi_image_free(pixels);

  image_create_options image_opts{};
  image_opts.width = width;
  image_opts.height = height;
  image_opts.format = VK_FORMAT_R8G8B8A8_SRGB;
  image_opts.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_opts.usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_opts.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  image_opts.image = &m_image;
  image_opts.image_memory = &m_image_memory;
}
