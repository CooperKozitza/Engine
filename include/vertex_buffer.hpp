#pragma once

#include "device.hpp"
#include "vertex.hpp"

#include <mutex>

namespace eng {
class vertex_buffer {
public:
  vertex_buffer();
  ~vertex_buffer();

  void create_vertex_buffer(device *dev);

  void set_vertices(std::vector<vertex> &vertices);
  void add_vertices(std::vector<vertex> &vertices);

  VkBuffer &get() { return m_vertex_buffer; };

  size_t get_vertex_count() { return m_vertices.size(); }

private:
  VkDevice m_device;

  VkBuffer m_vertex_buffer;
  VkDeviceMemory m_vertex_buffer_memory;

  std::vector<vertex> m_vertices;

  std::mutex m_creation_mutex;
};
}