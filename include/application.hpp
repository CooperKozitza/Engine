#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#include "buffers.hpp"
#include "device.hpp"
#include "framebuffer.hpp"
#include "graphics_pipeline.hpp"
#include "instance.hpp"
#include "object.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "swap_chain.hpp"
#include "window.hpp"
#include "vertex.hpp"

namespace eng {
class renderer;

constexpr uint32_t DESCRIPTOR_POOL_OVERALLOCATION = 10;

class application {
public:
  static application &create(const glm::uvec2 &res, const char *title, bool fullscreen = false);
  static application &get() { return app; }

  application(const application &) = delete;
  const application &operator=(const application &) = delete;

  ~application();

  void add_shader(const char *file_path, shader_type type);

  template <typename T, typename... Args> object &instantiate(Args &&...args);

  size_t get_object_count() { return m_objects.size(); }
  std::vector<std::unique_ptr<object>> &get_objects() { return m_objects; }

  void start();
  void stop();

  bool is_running();

  double delta_time() { return m_delta_time.count(); }

private:
  application();
  static application app;

  void initialize_rendering();

  // prevents two threads from simultaneously creating an application
  // instance
  static std::mutex creation_mutex;

  std::mutex m_instantiation_mutex;

  // flag for whether or not the application is running (used to stop thread)
  std::atomic<bool> m_is_running;

  std::thread m_update_thread;

  const char *m_name;
  window_details m_window_details;

  std::unique_ptr<renderer> m_renderer;

  std::vector<std::unique_ptr<object>> m_objects;

  std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update_end;
  std::chrono::duration<double> m_delta_time;
};

class renderer {
public:
  renderer();
  virtual ~renderer() = default;

  renderer(const renderer &) = delete;
  void operator=(const renderer &) = delete;

  virtual void render_frame() = 0;
  virtual double delta_time() = 0;

  virtual void add_shader(const char *file_path, shader_type type) = 0;

  virtual void start_rendering(window_details &window_details) = 0;
  virtual void stop_rendering() = 0;

  bool is_rendering() { return m_is_running; }

protected:
  application &m_application;
  uint32_t m_current_frame;

  std::thread m_rendering_thread;

  std::atomic<bool> m_should_close;
  std::atomic<bool> m_is_running;
};

class vulkan_renderer : public renderer {
public:
  vulkan_renderer();
  ~vulkan_renderer();

  static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  void render_frame();
  virtual double delta_time();

  virtual void start_rendering(window_details &window_details);
  virtual void stop_rendering();

  void add_shader(const char *file_path, shader_type type) {
    m_pipeline->add_shader(file_path, type);
  }

private:
  void initialize_rendering(window_details &window_details);

  std::unique_ptr<window> m_window;

  std::unique_ptr<instance> m_instance;
  std::unique_ptr<surface> m_surface;
  std::unique_ptr<device> m_device;

  std::unique_ptr<swap_chain> m_swap_chain;
  std::unique_ptr<pipeline> m_pipeline;
  std::unique_ptr<framebuffer> m_framebuffer;

  std::unique_ptr<command_pool> m_command_pool;

  std::unique_ptr<descriptor_pool> m_descriptor_pool;

  // synchronization

  std::vector<VkSemaphore> image_available_semaphores;
  std::vector<VkSemaphore> render_finished_semaphores;
  std::vector<VkFence> in_flight_fences;

  std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update_end;
  std::chrono::duration<double> m_delta_time;
};

template <typename T, typename... Args>
inline object &application::instantiate(Args &&...args) {
  static_assert(std::is_base_of<object, T>::value,
                "The object passed was not derived from eng::object");
  std::lock_guard<std::mutex> guard(m_instantiation_mutex);

  object &instantiated_object =
      *m_objects.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));

  return instantiated_object;
}
} // namespace eng