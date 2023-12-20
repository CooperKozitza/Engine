#include "../include/object.hpp"
#include "../include/application.hpp"

eng::object::object() : m_position(), m_rotation() {
  m_vertex_buffer = std::make_unique<vertex_buffer>();
  m_uniform_buffer = std::make_unique<uniform_buffer>();
}

eng::object::~object() {}

void eng::object::initialize_rendering(device &dev, command_pool &cmd_pool) {
  m_vertex_buffer->create_vertex_buffer(dev, cmd_pool);
  m_vertex_buffer->create_index_buffer(dev, cmd_pool);

  m_uniform_buffer->create_uniform_buffer(dev);
}

void eng::object::uninitialize_rendering() {
  m_vertex_buffer.reset();
  m_uniform_buffer.reset();
}

void eng::object::set_position(glm::vec3 position) {
  std::lock_guard<std::mutex> guard(m_transformation_mutex);

  m_position = position;
}

void eng::object::set_rotation(glm::vec3 rotation) {
  std::lock_guard<std::mutex> guard(m_transformation_mutex);

  m_rotation = rotation;
}