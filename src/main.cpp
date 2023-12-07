#include "../include/application.hpp"

int main() {
  eng::application *const app = eng::application::create({800, 600}, "Test!");

  app->set_shader("C:/Users/coope/Documents/Engine/shaders/test.vert",
                  eng::VERTEX);
  app->set_shader("C:/Users/coope/Documents/Engine/shaders/test.frag",
                  eng::FRAGMENT);

  app->start();
  while (app->is_running()) {

  }
  app->stop();

  delete app;
}