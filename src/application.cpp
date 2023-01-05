#include "../include/application.hpp"

eng::application eng::application::app;

std::mutex eng::application::creation_mutex;

eng::application &eng::application::create(const glm::uvec2 &res,
                                           const char *title, bool fullscreen) {
  std::lock_guard<std::mutex> guard(creation_mutex);
  app.m_name = title;
  app.m_window_details.title = title;
  app.m_window_details.resolution = res;
  app.m_window_details.fullscreen = fullscreen;

  app.initialize_rendering();

  return app;
}

eng::application::application()
    : m_window_details(), m_name(), m_objects(), m_delta_time(),
      m_last_update_end() {}

void eng::application::initialize_rendering() {
  m_renderer = std::make_unique<vulkan_renderer>();
}

eng::application::~application() {
  if (is_running()) {
    stop();
    m_renderer->stop_rendering();
  }

  m_renderer.reset();
}

void eng::application::add_shader(const char *file_path, shader_type type) {
  m_renderer->add_shader(file_path, type);
}

void eng::application::start() {
  for (std::unique_ptr<object> &obj : m_objects) {
    obj->start();
  }

  m_is_running = true;
  m_renderer->start_rendering(m_window_details);

  m_update_thread = std::thread([this] {
    const auto min_update_time =
        std::chrono::milliseconds(1000) / 900; // 900 Hz

    while (is_running()) {
      for (std::unique_ptr<object> &obj : m_objects) {
        obj->update(m_delta_time.count());
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> now =
          std::chrono::high_resolution_clock::now();
      m_delta_time = now - m_last_update_end;
      m_last_update_end = now;

      if (m_delta_time < min_update_time) {
        std::chrono::duration<double, std::milli> sleep_time =
            min_update_time - m_delta_time;

        std::this_thread::sleep_for(sleep_time);
      }

      now = std::chrono::high_resolution_clock::now();
      m_delta_time = now - m_last_update_end;
      m_last_update_end = now;
    }

    m_is_running = false;
  });
}

void eng::application::stop() {
  m_renderer->stop_rendering();

  m_is_running = false;
  if (m_update_thread.joinable()) {
    m_update_thread.join();
  }
}

bool eng::application::is_running() {
  return m_renderer->is_rendering() && m_is_running;
}

eng::renderer::renderer()
    : m_current_frame(), m_should_close(false), m_is_running(false),
      m_application(application::get()) {}

eng::vulkan_renderer::vulkan_renderer() : renderer(), m_delta_time() {
  m_instance = std::make_unique<instance>();
  m_surface = std::make_unique<surface>();
  m_device = std::make_unique<device>();

  m_swap_chain = std::make_unique<swap_chain>();
  m_pipeline = std::make_unique<pipeline>();
  m_framebuffer = std::make_unique<framebuffer>();

  m_command_pool = std::make_unique<command_pool>();

  m_descriptor_pool = std::make_unique<descriptor_pool>();

  image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

  m_last_update_end = std::chrono::high_resolution_clock::now();
}

eng::vulkan_renderer::~vulkan_renderer() {
  if (m_is_running) {
    stop_rendering();
  }

  vkDeviceWaitIdle(m_device->get_device());

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(m_device->get_device(), image_available_semaphores[i],
                       nullptr);
    vkDestroySemaphore(m_device->get_device(), render_finished_semaphores[i],
                       nullptr);
    vkDestroyFence(m_device->get_device(), in_flight_fences[i], nullptr);
  }

  m_descriptor_pool.reset();
  m_command_pool.reset();

  m_framebuffer.reset();
  m_pipeline.reset();
  m_swap_chain.reset();

  for (std::unique_ptr<object> &obj : m_application.get_objects()) {
    obj->uninitialize_rendering();
    obj.reset();
  }

  m_device.reset();
  m_surface.reset();
  m_instance.reset();
}

void eng::vulkan_renderer::initialize_rendering(
    window_details &window_details) {
  m_window = std::make_unique<window>(window_details);

  m_instance->create_instance(window_details.title);
  m_surface->create_surface(*m_instance, *m_window);
  m_device->create_device(*m_instance, *m_surface);

  m_swap_chain->create_swap_chain(*m_device, *m_surface, *m_window);
  m_swap_chain->create_image_views(*m_device);

  m_pipeline->create_render_pass(*m_device, *m_swap_chain);
  m_pipeline->create_descriptor_set_layout(*m_device);
  m_pipeline->create_graphics_pipeline(*m_device, *m_swap_chain);

  m_framebuffer->create_framebuffers(*m_device, *m_swap_chain, *m_pipeline);

  m_command_pool->create_command_pool(*m_device);
  m_command_pool->create_command_buffers(*m_device, MAX_FRAMES_IN_FLIGHT);

  uint32_t object_count =
      static_cast<uint32_t>(m_application.get_object_count());
  m_descriptor_pool->create_descriptor_pool(
      *m_device, object_count + DESCRIPTOR_POOL_OVERALLOCATION);

  // synchronization object creation

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_device->get_device(), &semaphore_info, nullptr,
                          &image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(m_device->get_device(), &semaphore_info, nullptr,
                          &render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(m_device->get_device(), &fence_info, nullptr,
                      &in_flight_fences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }

  for (std::unique_ptr<object> &obj : m_application.get_objects()) {
    obj->initialize_rendering(*m_device, *m_command_pool);
  }

  m_descriptor_pool->create_descriptor_sets(*m_device, *m_pipeline,
                                            m_application.get_objects());
}

void eng::vulkan_renderer::render_frame() {
  VkDevice device = m_device->get_device();

  // wait for previous frame to finish by blocking execution
  vkWaitForFences(device, 1, &in_flight_fences[m_current_frame], VK_TRUE,
                  UINT64_MAX);
  vkResetFences(device, 1, &in_flight_fences[m_current_frame]);

  VkCommandBuffer command_buffer =
      m_command_pool->get_command_buffer(m_current_frame);

  // update the descriptor sets for this frame to ensure uniform buffers are
  // correctly represente in the gpu
  m_descriptor_pool->update_descriptor_sets(
      *m_device, *m_pipeline, m_application.get_objects(), m_current_frame);

  // get the image from the swap chain
  uint32_t image_index;
  vkAcquireNextImageKHR(device, m_swap_chain->get_swapchain(), UINT64_MAX,
                        image_available_semaphores[m_current_frame],
                        VK_NULL_HANDLE, &image_index);

  // reset then record the command buffer for this frame
  vkResetCommandBuffer(command_buffer, 0);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = m_pipeline->get_render_pass();
  render_pass_info.framebuffer = m_framebuffer->get_framebuffer(image_index);
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = m_swap_chain->get_extent();

  VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f}}};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline->get_pipeline());

  VkViewport viewport{};
  viewport.x = 0.0f, viewport.y = 0.0f;
  viewport.minDepth = 0.0f, viewport.maxDepth = 1.0f;
  viewport.width = (float)m_swap_chain->get_extent().width;
  viewport.height = (float)m_swap_chain->get_extent().height;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = m_swap_chain->get_extent();
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  const float aspect_ratio = static_cast<float>(scissor.extent.width) /
                             static_cast<float>(scissor.extent.height);

  const glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
  const glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
  const glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);

  uniform_buffer_object ubo{};
  ubo.proj = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);
  ubo.view = glm::lookAt(camera_pos, camera_target, up_vector);

  size_t descriptor_set_index = 0;
  for (std::unique_ptr<object> &obj : m_application.get_objects()) {
    vertex_buffer *vb = obj->get_vertex_buffer();
    uniform_buffer *ub = obj->get_uniform_buffer();

    glm::vec3 rot = obj->get_rotation();
    ubo.model = glm::translate(glm::mat4(1.0f), obj->get_position()) *
                glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1, 0, 0)) *
                glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0, 1, 0)) *
                glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0, 0, 1));

    ub->update_uniform_buffer(m_current_frame, ubo);

    VkBuffer vertex_buffers[] = {vb->get_vertex_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

    VkBuffer index_buffer = vb->get_index_buffer();
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);

    VkPipelineLayout pipeline_layout = m_pipeline->get_pipeline_layout();
    VkDescriptorSet descriptor_set = m_descriptor_pool->get_descriptor_set(
        descriptor_set_index +
        (m_current_frame * m_application.get_object_count()));
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

    uint32_t indices = static_cast<uint32_t>(vb->get_index_count());
    vkCmdDrawIndexed(command_buffer, indices, 1, 0, 0, 0);

    ++descriptor_set_index;
  }

  vkCmdEndRenderPass(command_buffer);

  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {image_available_semaphores[m_current_frame]};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  VkSemaphore signal_semaphores[] = {
      render_finished_semaphores[m_current_frame]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  if (vkQueueSubmit(m_device->get_graphics_queue(), 1, &submit_info,
                    in_flight_fences[m_current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swap_chains[] = {m_swap_chain->get_swapchain()};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;

  VkResult result =
      vkQueuePresentKHR(m_device->get_present_queue(), &present_info);

  // reset framebuffer and swap chain if the window is resized
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    vkDeviceWaitIdle(device);

    m_framebuffer = std::make_unique<framebuffer>();
    m_swap_chain = std::make_unique<swap_chain>();

    m_swap_chain->create_swap_chain(*m_device, *m_surface, *m_window);
    m_swap_chain->create_image_views(*m_device);

    m_framebuffer->create_framebuffers(*m_device, *m_swap_chain, *m_pipeline);

  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
  m_delta_time = std::chrono::high_resolution_clock::now() - m_last_update_end;
  m_last_update_end = std::chrono::high_resolution_clock::now();
}

double eng::vulkan_renderer::delta_time() { return m_delta_time.count(); }

void eng::vulkan_renderer::start_rendering(window_details &window_details) {
  if (!m_pipeline->required_shaders_set()) {
    std::cerr
        << "Vertex and Fragment Shaders Not Set! Set Shaders With "
           "application::set_shader(...) Before Calling application::start()"
        << std::endl;
    return;
  }

  m_is_running = true;
  m_rendering_thread = std::thread([this, &window_details] {
    initialize_rendering(window_details);

    while (!m_window->should_close() && !m_should_close) {
      m_window->poll_events();

      render_frame();
    }

    m_is_running = false;
  });
}

void eng::vulkan_renderer::stop_rendering() {
  m_should_close = true;
  if (m_rendering_thread.joinable()) {
    m_rendering_thread.join();
  }
}