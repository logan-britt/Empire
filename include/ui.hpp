#ifndef UI 
#define UI

#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include <vector>
#include <string>

namespace ui
{
  struct Button_Init
  {
    std::string text;
    float x, y;
    float width, height;
  };
  struct Button
  {
    float x, y;
    float width, height;
    std::string title;
    merlin::Graph render_graph;
  };

  Button* create_button(Button_Init init, merlin::Window* window);
  void destroy_button(Button* button);

  void activate_graphics(Button* button);
  void deactivate_graphics(Button* button);
  bool inside(float x, float y, Button* button);

  struct Panel_Init
  {
  };
  struct Panel
  {
  };

  //Panel* create_panel(Panel_Init init, merlin::Window* window);
  //void destroy_panel(Panel* panel);
}

#endif