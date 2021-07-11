#include "../include/merlin.hpp"
#include "../include/merlin_draw.hpp"

#include "../include/ui.hpp"

#include "../include/place.hpp"
#include "../include/place_building.hpp"
#include "../include/place_people.hpp"
#include "../include/place_resorces.hpp"

#include "../libs/SDL2/include/SDL.h"

#include <iostream>
#include <thread>
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

uint32_t click_state;
float mouse_x, mouse_y;

uint32_t cpu_count;
uint32_t current_thread_count;
uint32_t thread_max_count;





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
  cpu_count = std::thread::hardware_concurrency();
  if(cpu_count % 2 == 0) {
    thread_max_count = cpu_count/2;
  }
  else {
    thread_max_count = cpu_count/2 + 1;
  }
  current_thread_count = 1;

  merlin::init();
  merlin::Engine_Init e_init = {true, DISCRETE};
  merlin::Window_Init w_init = {1280, 720, "Empire", false};
  merlin::jump_engine(w_init, &window, e_init, &g_engine);

  game_quit = false;
  draw_targets = {};
  load_main_menu();

  int _mouse_x, _mouse_y;

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
    click_state = SDL_GetMouseState(&_mouse_x, &_mouse_y);
    mouse_x = (float)_mouse_x;
    mouse_y = (float)_mouse_y;
    
    /* --- render the simulation data --- */
    merlin::draw(draw_targets);
  }while(!game_quit);

  /* --- clean up the simulation --- */
  for(auto button : buttons) {
    ui::destroy_button(button);
  }

  merlin::destroy_engine(g_engine);
  merlin::terminate();
  return 0;
}





void main_menu_handel(SDL_Event event) {
  bool clicked = event.type == SDL_MOUSEBUTTONDOWN;

  for(auto button : buttons) {
    if(ui::inside(mouse_x, mouse_y, button) && clicked) {
      if(button->title == "Create World") {
        load_world_create();
      }
      else if(button->title == "Start Simulation") {
        load_simulation();
      }
      else if(button->title == "Load Simulation") {
        load_load();
      }
      else {
        game_quit = true;
      }
    }
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

  std::array<float, 3> color_normal = {0.1f, 0.1f, 1.0f};
  std::array<float, 3> color_heighlight = {0.1f, 0.55f, 1.0f};
  std::array<float, 3> color_gray = {0.0f, 0.0f, 0.0f};

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
  world_create_init.color_normal = color_normal;
  world_create_init.color_heighlight = color_heighlight;
  world_create_init.color_gray = color_gray;

  ui::Button* world_create_button = ui::create_button(world_create_init, &window);
  (world_create_button);
  buttons.push_back(world_create_button);
  //draw_targets.push_back(&world_create_button->render_graph);

  ui::Button_Init start_simulation_init = {};
  start_simulation_init.x = window_width/2.0f - button_width/2.0f;
  start_simulation_init.y = outer_gap + button_height + inner_gap;
  start_simulation_init.width = button_width;
  start_simulation_init.height = button_height;
  start_simulation_init.text = "Start Simulation";
  start_simulation_init.color_normal = color_normal;
  start_simulation_init.color_heighlight = color_heighlight;
  start_simulation_init.color_gray = color_gray;

  ui::Button* start_simultion_button = ui::create_button(start_simulation_init, &window);
  buttons.push_back(start_simultion_button);
  //draw_targets.push_back(&start_simultion_button->render_graph);

  ui::Button_Init load_simulation_init = {};
  load_simulation_init.x = window_width/2.0f - button_width/2.0f;
  load_simulation_init.y = outer_gap + 2.0f*button_height + 2.0f*inner_gap;
  load_simulation_init.width = button_width;
  load_simulation_init.height = button_height;
  load_simulation_init.text = "Load Simulation";
  load_simulation_init.color_normal = color_normal;
  load_simulation_init.color_heighlight = color_heighlight;
  load_simulation_init.color_gray = color_gray;

  ui::Button* load_simulation_button = ui::create_button(load_simulation_init, &window);
  buttons.push_back(load_simulation_button);
  //draw_targets.push_back(&load_simulation_button->render_graph);

  ui::Button_Init quit_button_init = {};
  quit_button_init.x = window_width/2.0f - button_width/2.0f;
  quit_button_init.y = outer_gap + 3.0f*button_height + 3.0f*inner_gap;
  quit_button_init.width = button_width;
  quit_button_init.height = button_height;
  quit_button_init.text = "Quit";
  quit_button_init.color_normal = color_normal;
  quit_button_init.color_heighlight = color_heighlight;
  quit_button_init.color_gray = color_gray;

  ui::Button* quit_button = ui::create_button(quit_button_init, &window);
  buttons.push_back(quit_button);
  //draw_targets.push_back(&quit_button->render_graph);
}
void load_world_create() {
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

  /* ---  --- */
}
void load_load() {
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

  /* ---  --- */
}
void load_simulation() {
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

  /* ---  --- */
}