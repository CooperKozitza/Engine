#pragma once

#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"

namespace eng {
class framebuffer {
public:
  framebuffer();
  ~framebuffer();

  void create_framebuffers(device *dev, swap_chain *sc, graphics_pipeline *gp);

  std::vector<VkFramebuffer> &get() { return m_framebuffers; };
  VkFramebuffer &get_framebuffer(size_t index) { return m_framebuffers[index]; };

private:
  VkDevice m_device;

  std::vector<VkFramebuffer> m_framebuffers;
};
} // namespace eng