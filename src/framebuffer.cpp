#include "../include/framebuffer.hpp"

void eng::framebuffer::create_framebuffers(device *dev, swap_chain *sc, graphics_pipeline *gp) {
  framebuffers.resize(sc->get_image_views().size());

  for (size_t i = 0; i < sc->get_image_views().size(); i++) {
    VkImageView attachments[] = {sc->get_image_views()[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = gp->get_render_pass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = sc->get_extent().width;
    framebufferInfo.height = sc->get_extent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(dev->get(), &framebufferInfo, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}