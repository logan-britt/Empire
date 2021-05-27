#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include "../include/place.hpp"
#include "../include/place_building.hpp"
#include "../include/place_people.hpp"
#include "../include/place_resorces.hpp"

#include "../libs/SDL2/include/SDL.h"
#undef main

enum state{MAIN_MENU, LOAD_MENU, WORLD_CREATE, START_GAME, QUIT};

int main() {
  /* --- set up the simulation engine --- */
  bool game_quit = false;
  state game_state = MAIN_MENU;

  merlin::init();
  merlin::Engine_Init e_init = {true, DISCRETE};
  merlin::Window_Init w_init = {1280, 720, "Empire", true};

  merlin::Engine g_engine; merlin::Window window;
  merlin::jump_engine(w_init, &window, e_init, &g_engine);

  merlin::Shader test_shader = {};
  test_shader.name = "main";
  test_shader.vertex_path = "shaders/triangle_vert.spv";
  test_shader.fragment_path = "shaders/triangle_frag.spv";
  test_shader.geometry = false;

  merlin::State_Init test_state = {};
  test_state.id = 0;
  test_state.shader = test_shader;

  merlin::Graph_Init test_graph_init = {};
  test_graph_init.active = test_state;
  test_graph_init.loaded = {};
  test_graph_init.unloaded = {};

  merlin::Graph test_graph = merlin::create_graph(test_graph_init);

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
  }while(!game_quit);

  /* --- clean up the simulation --- */
  merlin::destory_window(window);
  merlin::destroy_engine(g_engine);
  merlin::terminate();
  return 0;
}