#include "../include/vertex_buffer.hpp"

eng::vertex_buffer::vertex_buffer()
    : m_device(VK_NULL_HANDLE), m_vertex_buffer(VK_NULL_HANDLE),
      m_creation_mutex() {}

eng::vertex_buffer::~vertex_buffer() {
  if (m_device != VK_NULL_HANDLE && m_vertex_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);
  }
}

void eng::vertex_buffer::create_vertex_buffer(device *dev) {
  std::lock_guard<std::mutex> guard(m_creation_mutex);

  m_device = dev->get();

  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = sizeof(m_vertices[0]) * m_vertices.size();
  buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_device, &buffer_info, nullptr, &m_vertex_buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, m_vertex_buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = dev->find_memory_type(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(m_device, &allocInfo, nullptr,
                       &m_vertex_buffer_memory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(m_device, m_vertex_buffer, m_vertex_buffer_memory, 0);

  void *data;
  vkMapMemory(m_device, m_vertex_buffer_memory, 0, buffer_info.size, 0, &data);
  memcpy(data, m_vertices.data(), (size_t)buffer_info.size);
  vkUnmapMemory(m_device, m_vertex_buffer_memory);
}

void eng::vertex_buffer::set_vertices(std::vector<vertex> &vertices) {
  std::lock_guard<std::mutex> guard(m_creation_mutex);

  m_vertices.clear();
  for (vertex vertex : vertices) {
    m_vertices.push_back(vertex);
  }
}

void eng::vertex_buffer::add_vertices(std::vector<vertex> &vertices) {
  std::lock_guard<std::mutex> guard(m_creation_mutex);

  for (vertex vertex : vertices) {
    m_vertices.push_back(vertex);
  }
}
