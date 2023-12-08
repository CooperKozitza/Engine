#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "vectors.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "graphics_pipeline.hpp"
#include "framebuffer.hpp"
#include "command_buffer.hpp"

namespace eng {
class application {
public:
  static application *create(const vec2<uint32_t> &res, const char *title);
  static application *get() { return app; }

  application(const application &) = delete;
  const application &operator=(const application &) = delete;

  ~application();

  void set_shader(const char *file_path, shader_type type);

  void start();
  void stop();

  bool is_running() { return m_running; }

private:
  application(const vec2<uint32_t> res, const char *name);
  static application *app;

  // helper funcs:
  void create_sync_objects();
  void draw_frame(command_buffer_options &command_buff_opts);

  // prevents two threads from simultaneously creating an application
  // instance
  static std::mutex creation_mutex;

  // flag for whether or not the application is running (used to stop thread)
  std::atomic<bool> m_running;

  // the thread where the main loop runs, defined in start()
  std::thread m_main;

  // the name of the application
  const char *m_name;

  // the glfw window instance
  window_details m_window_details;
  window *m_window;

  // the vulkan instance
  instance *m_instance;

  // the vulkan surface
  surface *m_surface;

  // the vulkan device
  device *m_device;

  // the vulkan swap chain
  swap_chain *m_swap_chain;

  // the vulkan graphics pipeline and render pass
  graphics_pipeline *m_graphics_pipeline;

  // the swap chain framebuffer
  framebuffer *m_framebuffer;

  // the command pool and command buffer
  command_buffer *m_command_buffer;

  // synchronization
  VkSemaphore image_available_semaphore;
  VkSemaphore render_finished_semaphore;
  VkFence in_flight_fence;
};
} // namespace eng