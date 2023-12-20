#include "../include/application.hpp"

class my_object : public eng::object {
  void start() {
    std::vector<eng::vertex> verts = {{{-10.0f, -10.0f}, {1.0f, 0.0f, 0.0f}},
                                      {{-10.0f, 10.0f}, {0.0f, 1.0f, 0.0f}},
                                      {{10.0f, 10.0f}, {0.0f, 0.0f, 1.0f}},
                                      {{10.0f, -10.0f}, {1.0f, 1.0f, 0.0f}}};
    std::vector<uint16_t> indices = {0, 1, 3, 2};

    set_position({50.0f, 50.0f, 1.0f});

    get_vertex_buffer()->set_vertices(verts);
    get_vertex_buffer()->set_indices(indices);
  }

  void update(double dt) {
    
  }
};

int main() {
  eng::application &app = eng::application::create({800, 600}, "Test!");

  app.add_shader("C:/Users/coope/Documents/Engine/shaders/vert.spv",
                  eng::VERTEX);
  app.add_shader("C:/Users/coope/Documents/Engine/shaders/frag.spv",
                  eng::FRAGMENT);

  eng::object &obj = app.instantiate<my_object>();

  app.start();
  while (app.is_running()) {
  }
  app.stop();
}