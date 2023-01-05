#include "../include/application.hpp"

class my_object : public eng::object {
  void start() {
    std::vector<eng::vertex> verts = {
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},  // Front-top-left
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   // Front-top-right
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}, // Back-top-left
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},  // Back-top-right
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},   // Front-bottom-left
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},    // Front-bottom-right
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},  // Back-bottom-left
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}    // Back-bottom-right
    };

    // Define the indices for each triangle of the cube
    std::vector<uint16_t> indices = {
        0, 4, 1, 1, 4, 5,
        2, 6, 3, 3, 6, 7,
        0, 1, 2, 2, 1, 3,
        4, 6, 5, 5, 6, 7,
        0, 2, 4, 4, 2, 6,
        1, 5, 3, 3, 5, 7};

    get_vertex_buffer()->set_vertices(verts);
    get_vertex_buffer()->set_indices(indices);
  }

  void update(double dt) {
    set_rotation(get_rotation() + glm::vec3{1.0f * dt, 1.0f * dt, 1.0f * dt});
  }
};

int main() {
  eng::application &app = eng::application::create({1600, 1000}, "Test!", true);

  app.add_shader("C:/Users/coope/Documents/Engine/shaders/vert.spv",
                 eng::VERTEX);
  app.add_shader("C:/Users/coope/Documents/Engine/shaders/frag.spv",
                 eng::FRAGMENT);

  eng::object &obj = app.instantiate<my_object>();

  app.start();

  while (app.is_running()) {}

  app.stop();
}