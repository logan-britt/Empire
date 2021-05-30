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