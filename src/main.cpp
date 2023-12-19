#include "../include/application.hpp"

class my_object : public eng::object {
  void start() {
    std::vector<eng::vertex> verts = {{{0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}};
    std::vector<uint16_t> indices = {0};

    for (short i = 0; i < 10; ++i) {
      float x = cos(i * (10 / 6.283f)), y = sin(i * (10 / 6.283f));
      verts.push_back({{x, y}, {1.0f, 0.0f, 1.0f}});

      indices.push_back(verts.size());
      indices.push_back(0);
    }

    get_vertex_buffer()->set_vertices(verts);
    get_vertex_buffer()->set_indices(indices);
  }

  void update(double dt) {
    set_rotation(get_rotation() + glm::vec3{0.00001f * dt, 0.00001f * dt, 0.0f});
  }
};

int main() {
  eng::application *const app = eng::application::create({800, 600}, "Test!");

  app->add_shader("C:/Users/coope/Documents/Engine/shaders/vert.spv",
                  eng::VERTEX);
  app->add_shader("C:/Users/coope/Documents/Engine/shaders/frag.spv",
                  eng::FRAGMENT);

  my_object obj;

  app->start();
  while (app->is_running()) {
  }
  app->stop();

  delete app;
}