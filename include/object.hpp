#pragma once

#include <mutex>

#include "buffers.hpp"

namespace eng {
class object {
public:
  object();
  ~object();

  virtual void start() = 0;
  virtual void update() = 0;

  void set_position(glm::vec3 position);
  glm::vec3 get_position() { return m_position; }
  
  void set_rotation(glm::vec3 rotation)
  glm::vec3 get_rotation() { return m_rotation; }

private:
  std::mutex m_transformation_mutex;

  glm::vec3 m_position;
  glm::vec3 m_rotation;

  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<uniform_buffer> m_uniform_buffer;
};
} // namespace eng
