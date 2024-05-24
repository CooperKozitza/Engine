#include "../include/buffers.hpp"
#include "../include/application.hpp"
#include "../include/object.hpp"

eng::command_pool::command_pool()
    : m_device(VK_NULL_HANDLE), m_command_pool(VK_NULL_HANDLE),
      m_command_buffers() {}

eng::command_pool::~command_pool() {
  if (m_device != VK_NULL_HANDLE && m_command_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  }
}

void eng::command_pool::create_command_pool(device &dev) {
  m_device = dev.get_device();

  queue_family_indices queue_family_indices = dev.get_queue_family_indices();

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

  if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics command pool!");
  }
}

void eng::command_pool::create_command_buffers(device &dev, uint32_t count) {
  m_device = dev.get_device();

  m_command_buffers.resize(count);

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = m_command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)m_command_buffers.size();

  if (vkAllocateCommandBuffers(m_device, &alloc_info,
                               m_command_buffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

VkCommandBuffer eng::command_pool::start_one_time_command(device &dev) {
  m_device = dev.get_device();

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = m_command_pool;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void eng::command_pool::end_one_time_command(device &dev,
                                             VkCommandBuffer command_buffer) {
  m_device = dev.get_device();

  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;

  VkQueue graphics_queue = dev.get_graphics_queue();

  vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
}

eng::vertex_buffer::vertex_buffer()
    : m_index_buffer_size(), m_vertex_buffer_size(), m_device(VK_NULL_HANDLE),
      m_vertex_buffer(VK_NULL_HANDLE), m_index_buffer(VK_NULL_HANDLE),
      m_vertex_buffer_memory(VK_NULL_HANDLE),
      m_index_buffer_memory(VK_NULL_HANDLE), m_mutex() {}

eng::vertex_buffer::~vertex_buffer() {
  if (m_device != VK_NULL_HANDLE && m_vertex_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);
  }
  if (m_device != VK_NULL_HANDLE && m_index_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device, m_index_buffer, nullptr);
    vkFreeMemory(m_device, m_index_buffer_memory, nullptr);
  }
}

void eng::vertex_buffer::create_vertex_buffer(device &dev,
                                              command_pool &cmd_pool) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_device = dev.get_device();

  VkDeviceSize buffer_size = sizeof(m_vertices[0]) * m_vertices.size();

  VkBuffer staging_buffer{};
  VkDeviceMemory staging_buffer_memory{};

  buffer_create_options staging_buffer_opts{};
  staging_buffer_opts.size = buffer_size;
  staging_buffer_opts.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  staging_buffer_opts.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  staging_buffer_opts.buffer = &staging_buffer;
  staging_buffer_opts.buffer_memory = &staging_buffer_memory;

  dev.create_buffer(staging_buffer_opts);

  void *data;
  vkMapMemory(m_device, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, m_vertices.data(), (size_t)buffer_size);
  vkUnmapMemory(m_device, staging_buffer_memory);

  buffer_create_options vertex_buffer_opts{};
  vertex_buffer_opts.size = buffer_size;
  vertex_buffer_opts.usage =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  vertex_buffer_opts.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  vertex_buffer_opts.buffer = &m_vertex_buffer;
  vertex_buffer_opts.buffer_memory = &m_vertex_buffer_memory;

  dev.create_buffer(vertex_buffer_opts);

  dev.copy_buffer(cmd_pool, staging_buffer, m_vertex_buffer, buffer_size);

  vkDestroyBuffer(m_device, staging_buffer, nullptr);
  vkFreeMemory(m_device, staging_buffer_memory, nullptr);

  m_vertex_buffer_size = m_vertices.size();
}

void eng::vertex_buffer::update_vertex_buffer(device &dev,
                                              command_pool &cmd_pool) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_device = dev.get_device();

  if (m_vertices.size() > m_vertex_buffer_size) {
    // New data is larger than the current buffer, recreate buffer
    create_vertex_buffer(dev, cmd_pool);
  } else {
    void *data;
    vkMapMemory(m_device, m_vertex_buffer_memory, 0,
                sizeof(vertex) * m_vertices.size(), 0, &data);
    memcpy(data, m_vertices.data(), sizeof(vertex) * m_vertices.size());
    vkUnmapMemory(m_device, m_vertex_buffer_memory);
  }
}

void eng::vertex_buffer::create_index_buffer(device &dev,
                                             command_pool &cmd_pool) {
  m_device = dev.get_device();

  VkDeviceSize buffer_size = sizeof(m_indices[0]) * m_indices.size();

  VkBuffer staging_buffer{};
  VkDeviceMemory staging_buffer_memory{};

  buffer_create_options opts{};
  opts.size = buffer_size;
  opts.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  opts.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  opts.buffer = &staging_buffer;
  opts.buffer_memory = &staging_buffer_memory;

  dev.create_buffer(opts);

  void *data;
  vkMapMemory(m_device, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, m_indices.data(), (size_t)buffer_size);
  vkUnmapMemory(m_device, staging_buffer_memory);

  opts = {};
  opts.size = buffer_size;
  opts.usage =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  opts.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  opts.buffer = &m_index_buffer;
  opts.buffer_memory = &m_index_buffer_memory;

  dev.create_buffer(opts);

  dev.copy_buffer(cmd_pool, staging_buffer, m_index_buffer, buffer_size);

  vkDestroyBuffer(m_device, staging_buffer, nullptr);
  vkFreeMemory(m_device, staging_buffer_memory, nullptr);

  m_index_buffer_size = m_indices.size();
}

void eng::vertex_buffer::update_index_buffer(device &dev,
                                             command_pool &cmd_pool) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_device = dev.get_device();

  if (m_indices.size() > m_index_buffer_size) {
    // New data is larger than the current buffer, recreate buffer
    create_index_buffer(dev, cmd_pool);
  } else {
    void *data;
    vkMapMemory(m_device, m_index_buffer_memory, 0,
                sizeof(uint16_t) * m_indices.size(), 0, &data);
    memcpy(data, m_indices.data(), sizeof(uint16_t) * m_indices.size());
    vkUnmapMemory(m_device, m_index_buffer_memory);
  }
}

void eng::vertex_buffer::set_vertices(const std::vector<vertex> &vertices) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_vertices.clear();
  for (const vertex &vertex : vertices) {
    m_vertices.push_back(vertex);
  }
}

void eng::vertex_buffer::set_indices(const std::vector<uint16_t> &indices) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_indices.clear();
  for (const uint16_t index : indices) {
    m_indices.push_back(index);
  }
}

eng::uniform_buffer::uniform_buffer()
    : m_device(VK_NULL_HANDLE), m_uniform_buffer(), m_uniform_buffer_memory(),
      m_uniform_buffer_mapped() {}

eng::uniform_buffer::~uniform_buffer() {
  if (m_device != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device, m_uniform_buffer, nullptr);
    vkFreeMemory(m_device, m_uniform_buffer_memory, nullptr);
  }
}

void eng::uniform_buffer::create_uniform_buffer(device &dev) {
  m_device = dev.get_device();
  VkDeviceSize buffer_size = sizeof(uniform_buffer_object);

  buffer_create_options opts{};
  opts.size = buffer_size;
  opts.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  opts.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  opts.buffer = &m_uniform_buffer;
  opts.buffer_memory = &m_uniform_buffer_memory;

  dev.create_buffer(opts);

  vkMapMemory(m_device, m_uniform_buffer_memory, 0, buffer_size, 0,
              &m_uniform_buffer_mapped);
}

void eng::uniform_buffer::update_uniform_buffer(uint32_t current_image,
                                                uniform_buffer_object &ubo) {
  std::lock_guard<std::mutex> guard(m_mutex);

  memcpy(m_uniform_buffer_mapped, &ubo, sizeof(ubo));
}

eng::descriptor_pool::descriptor_pool()
    : m_device(VK_NULL_HANDLE), m_descriptor_pool(VK_NULL_HANDLE) {}

eng::descriptor_pool::~descriptor_pool() {
  if (m_device != VK_NULL_HANDLE && m_descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
  }
}

void eng::descriptor_pool::create_descriptor_pool(device &dev,
                                                  uint32_t set_count) {
  m_device = dev.get_device();

  VkDescriptorPoolSize pool_size{};
  pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = set_count * vulkan_renderer::MAX_FRAMES_IN_FLIGHT;

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;

  pool_info.maxSets = set_count * vulkan_renderer::MAX_FRAMES_IN_FLIGHT;

  if (vkCreateDescriptorPool(m_device, &pool_info, nullptr,
                             &m_descriptor_pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void eng::descriptor_pool::create_descriptor_sets(
    device &dev, pipeline &gp,
    const std::vector<std::unique_ptr<object>> &objs) {
  m_device = dev.get_device();

  const size_t descriptor_set_count =
      static_cast<size_t>(objs.size() * vulkan_renderer::MAX_FRAMES_IN_FLIGHT);
  m_descriptor_sets.resize(descriptor_set_count);

  VkDescriptorSetLayout layout = gp.get_descriptor_set_layout();
  std::vector<VkDescriptorSetLayout> layouts(descriptor_set_count, layout);

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = m_descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(descriptor_set_count);
  alloc_info.pSetLayouts = layouts.data();

  if (vkAllocateDescriptorSets(m_device, &alloc_info,
                               m_descriptor_sets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < objs.size() * vulkan_renderer::MAX_FRAMES_IN_FLIGHT;
       ++i) {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer =
        objs[i % objs.size()]->get_uniform_buffer()->get_uniform_buffer();
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = m_descriptor_sets[i];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(m_device, 1, &descriptor_write, 0, nullptr);
  }
}

void eng::descriptor_pool::update_descriptor_sets(
    device &dev, pipeline &gp, const std::vector<std::unique_ptr<object>> &objs,
    uint32_t current_frame) {
  m_device = dev.get_device();

  if (objs.size() > m_descriptor_sets.size()) {
    size_t new_set_count = std::max(
        m_descriptor_sets.size() * 2,
        m_descriptor_sets.size() + (objs.size() - m_descriptor_sets.size()));

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

    create_descriptor_pool(dev, static_cast<uint32_t>(new_set_count) + 1);
  }

  for (size_t i = 0; i < objs.size(); ++i) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = objs[i]->get_uniform_buffer()->get_uniform_buffer();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(uniform_buffer_object);

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet =
        m_descriptor_sets[i + (current_frame * objs.size())];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(m_device, 1, &descriptor_write, 0, nullptr);
  }
}
