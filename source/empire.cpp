#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include "../include/ui.hpp"

#include "../include/place.hpp"
#include "../include/place_building.hpp"
#include "../include/place_people.hpp"
#include "../include/place_resorces.hpp"

#include "../libs/SDL2/include/SDL.h"
#undef main

// the gloabal declerations for simulation data
enum major_state{MAIN_MENU, WORLD_CREATE, LOAD, PLAY};

merlin::Engine g_engine;
merlin::Window window;

bool game_quit;
major_state simulation_major_state;
std::vector<merlin::Graph*> draw_targets;
std::vector<ui::Button*> buttons;
std::vector<ui::Panel*> panels;





// handel the inputs for each simulation state
void main_menu_handel(SDL_Event event);
void world_create_handel(SDL_Event event);
void load_handel(SDL_Event event);
void simulation_handel(SDL_Event event);

void load_main_menu();
void load_world_create();
void load_load();
void load_simulation();





int main() {
  /* --- set up the simulation engine --- */
  merlin::init();
  merlin::Engine_Init e_init = {false, DISCRETE};
  merlin::Window_Init w_init = {1280, 720, "Empire", true};
  merlin::jump_engine(w_init, &window, e_init, &g_engine);

  game_quit = false;
  draw_targets = {};
  load_main_menu();

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
          switch(simulation_major_state)
          {
            case MAIN_MENU:
              main_menu_handel(event);
              break;
            
            case WORLD_CREATE:
              world_create_handel(event);
              break;

            case LOAD:
              load_handel(event);
              break;

            case PLAY:
              simulation_handel(event);
              break;
          }
      }
    }

    /* --- update simulation data --- */
    
    /* --- render the simulation data --- */
    merlin::draw(draw_targets);
  }while(!game_quit);

  /* --- clean up the simulation --- */

  merlin::destory_window(window);
  merlin::destroy_engine(g_engine);
  merlin::terminate();
  return 0;
}





void main_menu_handel(SDL_Event event) {
  for(auto button : buttons) {
    
  }
}
void world_create_handel(SDL_Event event) {

}
void load_handel(SDL_Event event) {

}
void simulation_handel(SDL_Event event) {

}

void load_main_menu() {
  /* --- clean up the global data from the previous scene --- */
  if(!buttons.empty()) {
    for(auto button : buttons) {
      ui::destroy_button(button);
    }
  }
  if(!panels.empty()) {
    for(auto panel : panels) {
      
    }
  }

  /* --- set the simulation state to main menu --- */
  simulation_major_state = MAIN_MENU;

  float window_height = (float)window.height;
  float window_width = (float)window.width;

  float button_height = 0.1f*window_height;
  float button_width = 0.05f*window_width;

  float outer_gap = 0.2f*window_height;
  float inner_gap = (window_height - 2.0f*outer_gap)/3.0f;

  ui::Button_Init world_create_init = {};
  world_create_init.x = window_width/2.0f - button_width/2.0f;
  world_create_init.y = outer_gap;
  world_create_init.width = button_width;
  world_create_init.height = button_height;
  world_create_init.text = "Create World";

  ui::Button* world_create_button = ui::create_button(world_create_init, &window);
  ui::activate_graphics(world_create_button);

  buttons.push_back(world_create_button);
  draw_targets.push_back(&world_create_button->render_graph);

  //ui::Button_Init start_simulation_init = {};
  //ui::Button* start_simultion_button = ui::create_button(start_simulation_init, &window);

  //ui::Button_Init load_simulation_init = {};
  //ui::Button* load_simulation_button = ui::create_button(load_simulation_init, &window);

  //ui::Button_Init quit_button_init = {};
  //ui::Button* quit_button = ui::create_button(quit_button_init, &window);
}
void load_world_create() {

}
void load_load() {

}
void load_simulation() {

}