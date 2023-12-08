#include "../include/shader.hpp"

eng::shader::shader() : m_module(VK_NULL_HANDLE), m_file_path(nullptr) {}

eng::shader::shader(const char *m_file_path, shader_type type)
    : m_module(VK_NULL_HANDLE), m_file_path(m_file_path), m_type(type) {}

std::vector<char> eng::shader::read_file(const char *m_file_path) {
  std::ifstream file(m_file_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t file_size = (size_t)file.tellg();
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);

  file.close();

  return buffer;
}

VkShaderModule eng::shader::create_shader_module(device *dev) {
  std::vector<char> code = read_file(m_file_path);

  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(dev->get(), &create_info, nullptr, &m_module) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return m_module;
}

void eng::shader::destroy_shader_module(device *dev) {
  vkDestroyShaderModule(dev->get(), m_module, nullptr);
}

VkShaderModule eng::shader::get_shader_module() { return m_module; }
