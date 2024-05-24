#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "device.hpp"

namespace eng {
struct image_create_options {
  uint32_t width, height;
  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkImage *image;
  VkDeviceMemory *image_memory;
};

class image {
public:
  image();
  ~image();

  void create_image(device &dev, image_create_options &opts);

protected:
  VkDevice m_device;

  VkImage m_image;
  VkDeviceMemory m_image_memory;
};

class texture : public image {
public:
  void create_texture(device &dev, const char *file_path);

private:

};
} // namespace eng