#include "window.hpp"

int main() {
  eng::window *win = new eng::window({800, 600}, "Test");

  while (!win->should_close()) {
    win->poll_events();
  }

  delete win;
}