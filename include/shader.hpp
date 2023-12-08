#pragma once

#include <fstream>

#include "device.hpp"

namespace eng {
enum shader_type { FRAGMENT, VERTEX };
class shader {
public:
  shader();
  shader(const char *file_name, shader_type type);

  static std::vector<char> read_file(const char *file_name);

  shader_type get_type() const { return m_type; }

  VkShaderModule create_shader_module(device *device);
  void destroy_shader_module(device *device);
  VkShaderModule get_shader_module();

private:
  VkShaderModule m_module;

  const char *m_file_path;    

  shader_type m_type;
};
} // namespace eng