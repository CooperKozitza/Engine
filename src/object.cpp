#include "../include/object.hpp"
#include "../include/application.hpp"

eng::object::object() { 
  application::get()->add_object(*this);
  
  m_vertex_buffer = std::make_unique<vertex_buffer>();
  m_uniform_buffer = std::make_unique<uniform_buffer>();
}

void eng::object::set_position(glm::vec3 position) {
  std::lock_guard<std::mutex> guard(m_transformation_mutex);

  m_position = position;
}

void eng::object::set_rotation(glm::vec3 rotation) {
  std::lock_guard<std::mutex> guard(m_transformation_mutex);

  m_rotation = rotation;
}