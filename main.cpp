#include "application.hpp"

int main() {
  eng::application *app = eng::application::create({800, 600}, "Test!");

  app->start();
  while (app->is_running()) {
  }
  app->stop();

  delete app;
}