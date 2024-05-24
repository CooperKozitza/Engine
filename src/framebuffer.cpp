#include "../include/framebuffer.hpp"

eng::framebuffer::framebuffer() : m_device(VK_NULL_HANDLE), m_framebuffers() {}

eng::framebuffer::~framebuffer() {
  if (m_device != VK_NULL_HANDLE) {
    for (VkFramebuffer &framebuff : m_framebuffers) {
      if (framebuff != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(m_device, framebuff, nullptr);
      }
    }
  }
}

void eng::framebuffer::create_framebuffers(device &dev, swap_chain &sc,
                                           pipeline &gp) {
  m_device = dev.get_device();

  std::vector<VkImageView> &image_views = sc.get_image_views();
  m_framebuffers.resize(image_views.size());

  for (size_t i = 0; i < image_views.size(); i++) {
    VkImageView attachments[] = {image_views[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = gp.get_render_pass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = sc.get_extent().width;
    framebufferInfo.height = sc.get_extent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr,
                            &m_framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}