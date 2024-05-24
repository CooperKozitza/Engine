#pragma once

#include "vertex_buffer.hpp"

namespace eng {
struct command_buffer_options {
  graphics_pipeline *graphics_pipeline;
  swap_chain *swap_chain;
  framebuffer *framebuffer;
  vertex_buffer *vertex_buffer;
  color clear_color;
};
} // namespace eng