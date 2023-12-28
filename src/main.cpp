#include "../include/application.hpp"

class my_object : public eng::object {
  void start() {
    std::vector<eng::vertex> verts = {{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                                      {{-1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}},
                                      {{1.0f, 1.0f},   {0.0f, 0.0f, 1.0f}},
                                      {{1.0f, -1.0f},  {1.0f, 1.0f, 0.0f}}};
    std::vector<uint16_t> indices = {1, 2, 3, 1, 3, 0};

    get_vertex_buffer()->set_vertices(verts);
    get_vertex_buffer()->set_indices(indices);
  }

  void update(double dt) { 
    set_rotation(get_rotation() + glm::vec3{0.0f, 0.0f, 0.01f});
  }
};

int main() {
  eng::application &app = eng::application::create({800, 600}, "Test!");

  app.add_shader("C:/Users/coope/Documents/Engine/shaders/vert.spv",
                 eng::VERTEX);
  app.add_shader("C:/Users/coope/Documents/Engine/shaders/frag.spv",
                 eng::FRAGMENT);

  eng::object &obj1 = app.instantiate<my_object>();
  eng::object &obj2 = app.instantiate<my_object>();
  eng::object &obj3 = app.instantiate<my_object>();

  obj1.set_position({1.5f, 1.5f, -10.0f});
  obj2.set_position({0.0f, -1.5f, -10.0f});
  obj3.set_position({-1.5f, 1.5f, -10.0f});

  obj2.set_rotation({0.0f, 0.0f, 90.0f});
  obj3.set_rotation({0.0f, 0.0f, 45.0f});

  app.start();
  while (app.is_running()) {
  }
  app.stop();
}