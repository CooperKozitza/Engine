#pragma once

#include <mutex>

#include "buffers.hpp"

namespace eng {
class component {
public:
  virtual ~component() = default;

  
};

class object {
public:
  object();
  ~object();

  virtual void start() = 0;
  virtual void update(double dt) = 0;

  void initialize_rendering(device &dev, command_pool &cmd_pool);
  void uninitialize_rendering();

  void set_position(glm::vec3 position);
  glm::vec3 get_position() const { return m_position; }

  void set_rotation(glm::vec3 rotation);
  glm::vec3 get_rotation() const { return m_rotation; }

  vertex_buffer *get_vertex_buffer() { return m_vertex_buffer.get(); }
  uniform_buffer *get_uniform_buffer() { return m_uniform_buffer.get(); }

private:
  std::mutex m_transformation_mutex;

  glm::vec3 m_position;
  glm::vec3 m_rotation;

  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<uniform_buffer> m_uniform_buffer;
};
} // namespace eng
