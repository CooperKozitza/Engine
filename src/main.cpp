#include "../include/application.hpp"

int main() {
  eng::application *const app = eng::application::create({800, 600}, "Test!");

  app->add_shader("C:/Users/coope/Documents/Engine/shaders/vert.spv",
                  eng::VERTEX);
  app->add_shader("C:/Users/coope/Documents/Engine/shaders/frag.spv",
                  eng::FRAGMENT);

  app->start();
  while (app->is_running()) {

  }
  app->stop();

  delete app;
}