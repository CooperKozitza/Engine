#include "../include/application.hpp"

class my_object : public eng::object {
  void start() {
    std::vector<eng::vertex> verts = {{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                                      {{-1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                                      {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                                      {{1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}}};
    std::vector<uint16_t> indices = {1, 2, 3, 1, 3, 0};

    get_vertex_buffer()->set_vertices(verts);
    get_vertex_buffer()->set_indices(indices);
  }

  int a = 1;

  void update(double dt) {
    if (dt == 0.0f)
      return;

    set_rotation(get_rotation() + glm::vec3{0.0f, 0.0f, 0.00001f * dt * a});

    glm::vec3 r = get_rotation();
    if (r.z > 10.0f || r.z < -10.0f) {
      a *= -1;
    }
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