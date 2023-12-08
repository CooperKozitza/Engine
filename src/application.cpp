#include "../include/application.hpp"

eng::application *eng::application::app;

std::mutex eng::application::creation_mutex;

eng::application *eng::application::create(const vec2<uint32_t> &res,
                                           const char *title) {
  std::lock_guard<std::mutex> guard(creation_mutex);
  if (!app) {
    app = new application(res, title);
  }

  return app;
}

eng::application::application(const vec2<uint32_t> res, const char *name)
    : m_window_details(res, name), m_window(nullptr), m_name(name),
      current_frame(), m_instance(new instance()), m_surface(new surface()),
      m_device(new device()), m_swap_chain(new swap_chain()),
      m_graphics_pipeline(new graphics_pipeline()),
      m_framebuffer(new framebuffer()),
      m_command_buffers(new command_buffers()),
      m_vertex_buffer(new vertex_buffer()) {
  image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
}

void eng::application::initialize_vulkan() {
  m_instance->create_instance(m_name);

  m_surface->create_surface(m_instance, m_window);

  m_device->create_device(m_instance, m_surface);

  m_swap_chain->create_swap_chain(m_device, m_surface, m_window);
  m_swap_chain->create_image_views(m_device);

  m_graphics_pipeline->create_render_pass(m_device, m_swap_chain);
  m_graphics_pipeline->create_graphics_pipeline(m_device, m_swap_chain);

  m_framebuffer->create_framebuffers(m_device, m_swap_chain,
                                     m_graphics_pipeline);

  m_command_buffers->create_command_pool(m_device);
  m_command_buffers->create_command_buffers(m_device, MAX_FRAMES_IN_FLIGHT);

  std::vector<vertex> verts;

  verts.push_back({{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 0.5f}});
  verts.push_back({{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 0.5f}});
  verts.push_back({{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}});

  verts.push_back({{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 0.5f}});
  verts.push_back({{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}});
  verts.push_back({{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 0.5f}});

   verts.push_back({{-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f, 0.5f}});
  verts.push_back({{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}});
  verts.push_back({{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 0.5f}});

  verts.push_back({{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 0.5f}});
  verts.push_back({{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}});
  verts.push_back({{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 0.5f}});

  m_vertex_buffer->set_vertices(verts);
  m_vertex_buffer->create_vertex_buffer(m_device);

  create_sync_objects();
}

void eng::application::create_sync_objects() {
  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_device->get(), &semaphore_info, nullptr,
                          &image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(m_device->get(), &semaphore_info, nullptr,
                          &render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(m_device->get(), &fence_info, nullptr,
                      &in_flight_fences[i]) != VK_SUCCESS) {

      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

void eng::application::draw_frame(command_buffer_options &command_buff_opts) {
  // wait for previous frame to finish by blocking execution
  vkWaitForFences(m_device->get(), 1, &in_flight_fences[current_frame], VK_TRUE,
                  UINT64_MAX);
  vkResetFences(m_device->get(), 1, &in_flight_fences[current_frame]);

  // get the image from the swap chain
  uint32_t image_index;
  vkAcquireNextImageKHR(m_device->get(), m_swap_chain->get(), UINT64_MAX,
                        image_available_semaphores[current_frame],
                        VK_NULL_HANDLE, &image_index);

  // reset then record the command buffer for this frame
  vkResetCommandBuffer(m_command_buffers->get(current_frame), 0);

  m_command_buffers->record_command_buffer(command_buff_opts, image_index,
                                           current_frame);

  // submit the command buffer
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_command_buffers->get(current_frame);

  VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  if (vkQueueSubmit(m_device->get_graphics_queue(), 1, &submit_info,
                    in_flight_fences[current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swap_chains[] = {m_swap_chain->get()};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;

  VkResult result =
      vkQueuePresentKHR(m_device->get_present_queue(), &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreate_swap_chain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void eng::application::recreate_swap_chain() {
  vkDeviceWaitIdle(m_device->get());

  reset_swap_chain();

  m_swap_chain->create_swap_chain(m_device, m_surface, m_window);
  m_swap_chain->create_image_views(m_device);

  m_framebuffer->create_framebuffers(m_device, m_swap_chain,
                                     m_graphics_pipeline);
}

void eng::application::reset_swap_chain() {
  delete m_framebuffer;
  delete m_swap_chain;

  m_framebuffer = new framebuffer();
  m_swap_chain = new swap_chain();
}

eng::application::~application() {
  vkDeviceWaitIdle(m_device->get());

  if (is_running()) {
    stop();
  }

  delete m_vertex_buffer;
  delete m_command_buffers;
  delete m_framebuffer;
  delete m_graphics_pipeline;
  delete m_swap_chain;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(m_device->get(), image_available_semaphores[i], nullptr);
    vkDestroySemaphore(m_device->get(), render_finished_semaphores[i], nullptr);
    vkDestroyFence(m_device->get(), in_flight_fences[i], nullptr);
  }

  delete m_device;
  delete m_surface;
  delete m_instance;
  delete m_window;
}

void eng::application::set_shader(const char *file_path, shader_type type) {
  m_graphics_pipeline->set_shader(file_path, type);
}

void eng::application::start() {
  if (!m_graphics_pipeline->required_shaders_set()) {
    std::cerr
        << "Vertex and Fragment Shaders Not Set! Set Shaders With "
           "application::set_shader(...) Before Calling application::start()"
        << std::endl;
    return;
  }

  m_running = true;
  m_main = std::thread([this] {
    m_window = new window(m_window_details);

    initialize_vulkan();

    command_buffer_options opts{};
    opts.swap_chain = m_swap_chain;
    opts.graphics_pipeline = m_graphics_pipeline;
    opts.framebuffer = m_framebuffer;
    opts.vertex_buffer = m_vertex_buffer;
    opts.clear_color = {0.0f, 0.0f, 0.0f, 1.0f};

    unsigned int i = 0;
    while (!m_window->should_close() && is_running()) {
      m_window->poll_events();

      draw_frame(opts);

      // main loop logic
    }

    m_running = false;
  });
}

void eng::application::stop() {
  std::cout << "Application Stopped" << std::endl;

  m_running = false;
  if (m_main.joinable()) {
    m_main.join();
  }
}