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

  shader_type get_type() const { return type; }

  VkShaderModule create_shader_module(device *device);
  void destroy_shader_module(device *device);
  VkShaderModule get_shader_module();

private:
  VkShaderModule module;

  const char *file_path;    

  shader_type type;
};
} // namespace eng