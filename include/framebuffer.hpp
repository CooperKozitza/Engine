#pragma once

#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"

namespace eng {
class framebuffer {
public:
  framebuffer();
  ~framebuffer();

  void create_framebuffers(device *dev, swap_chain *sc, graphics_pipeline *gp);

  std::vector<VkFramebuffer> get() { return framebuffers; };
  VkFramebuffer get_framebuffer(size_t index) { return framebuffers[index]; };

private:
  std::vector<VkFramebuffer> framebuffers;
};
} // namespace eng