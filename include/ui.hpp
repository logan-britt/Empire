#ifndef UI 
#define UI

#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include <array>
#include <vector>
#include <string>

namespace ui
{
  struct Button_Init
  {
    std::string text;
    float x, y;
    float width, height;

    VmaPool* pool;
    VmaAllocator* allocator;

    std::array<float, 3> color_normal;
    std::array<float, 3> color_heighlight;
    std::array<float, 3> color_gray;
  };
  struct Button
  {
    float x, y;
    float width, height;
    std::string title;

    std::array<float, 3> color_normal;
    std::array<float, 3> color_heighlight;
    std::array<float, 3> color_gray;

    std::vector<float> verticies;
    std::vector<uint32_t> indecies;

    merlin::Graph render_graph;
  };

  Button* create_button(Button_Init init, merlin::Window* window);
  void destroy_button(Button* button);

  void set_to_normal(Button* button);
  void set_to_hightlight(Button* button);
  void set_to_gray(Button* button);

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