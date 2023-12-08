#include "../include/application.hpp"

eng::application *eng::application::app;

std::mutex eng::application::creation_mutex;

eng::application *eng::application::create(const vec2<unsigned int> &res,
                                           const char *title) {
  std::lock_guard<std::mutex> guard(creation_mutex);
  if (!app) {
    app = new application(res, title);
  }

  return app;
}

eng::application::application(const vec2<unsigned int> res, const char *name)
    : m_window_details(res, name), m_window(nullptr), name(name),
      m_instance(new instance()), m_surface(new surface()),
      m_device(new device()), m_swap_chain(new swap_chain()),
      m_graphics_pipeline(new graphics_pipeline()) {}

void eng::application::uninitialize_vulkan() {}

eng::application::~application() {
  if (is_running()) {
    stop();
  }

  delete m_graphics_pipeline;
  delete m_swap_chain;
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

  running = true;
  main = std::thread([this] {
    m_window = new window(m_window_details);

    m_instance->create_instance(name);

    m_surface->create_surface(m_instance, m_window);

    m_device->create_device(m_instance, m_surface);

    m_swap_chain->create_swap_chain(m_device, m_surface);
    m_swap_chain->create_image_views(m_device);

    m_graphics_pipeline->create_render_pass(m_device, m_swap_chain);
    m_graphics_pipeline->create_graphics_pipeline(m_device, m_swap_chain);

    while (!m_window->should_close() && is_running()) {
      m_window->poll_events();

      // main loop logic
    }

    running = false;
  });
}

void eng::application::stop() {
  std::cout << "Application Stopped" << std::endl;

  running = false;
  if (main.joinable()) {
    main.join();
  }
}