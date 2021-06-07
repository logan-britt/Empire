#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include "../include/place.hpp"
#include "../include/place_building.hpp"
#include "../include/place_people.hpp"
#include "../include/place_resorces.hpp"

#include "../libs/SDL2/include/SDL.h"
#undef main

int main() {
  /* --- set up the simulation engine --- */
  bool game_quit = false;
  std::vector<merlin::Graph*> draw_targets = {};

  merlin::init();
  merlin::Engine_Init e_init = {false, DISCRETE};
  merlin::Window_Init w_init = {1280, 720, "Empire", true};

  merlin::Engine g_engine; merlin::Window window;
  merlin::jump_engine(w_init, &window, e_init, &g_engine);

  merlin::Shader test_shader = {};
  test_shader.name = "main";
  test_shader.vertex_path = "shaders/triangle_vert.spv";
  test_shader.fragment_path = "shaders/triangle_frag.spv";
  test_shader.geometry = false;

  merlin::Input test_input = {};
  test_input.input_data = false;

  merlin::Fixed_Functions test_fixed = {};
  test_fixed.reuse = false;
  test_fixed.topology = merlin::TRIANGLE_LIST;
  test_fixed.mode = merlin::FILL;
  test_fixed.line_width = 1.0f;

  merlin::Uniform test_uniform = {};
  test_uniform.uniform = false;

  merlin::Attachment test_attachment = {};
  test_attachment.sample_count = 1;
  test_attachment.data_ops = merlin::CLEAR_STORE;
  test_attachment.stencil_ops = merlin::DONT_CARE;
  test_attachment._layouts = merlin::UNDEFINED_PRESENT;

  merlin::Subpass subpass = {};
  subpass.point = merlin::GRAPHICS;
  subpass.color_attachments = {0};

  merlin::Render_Pass test_render_pass = {};
  test_render_pass.attachments = {test_attachment};
  test_render_pass.subpasses = {subpass};
  test_render_pass.dependencies = {};

  merlin::State_Init test_state_init = {};
  test_state_init.id = 2;
  test_state_init.shader = test_shader;
  test_state_init.input = test_input;
  test_state_init.fixed_functions = test_fixed;
  test_state_init.uniform = test_uniform;
  test_state_init.render_pass = test_render_pass;

  merlin::Graph_Init test_graph_init = {};
  test_graph_init.activated = true;
  test_graph_init.active = test_state_init;
  merlin::Graph test_graph = merlin::create_graph(test_graph_init, &window);
  draw_targets.push_back(&test_graph);

  /* --- the main simulation loop --- */
  do
  {
    /* --- listen for inputs --- */
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type)
      {
        case SDL_QUIT:
          game_quit = true;
          break;

        default:
          continue;
      }
    }

    /* --- update simulation data --- */
    
    /* --- render the simulation data --- */
    merlin::draw(draw_targets);
  }while(!game_quit);

  /* --- clean up the simulation --- */
  merlin::destroy_graph(test_graph);

  merlin::destory_window(window);
  merlin::destroy_engine(g_engine);
  merlin::terminate();
  return 0;
}