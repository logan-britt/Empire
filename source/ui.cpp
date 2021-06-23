#include "../include/ui.hpp"

ui::Button* ui::create_button(Button_Init init, merlin::Window* window) {
  Button* button = new Button;
  button->x = init.x;
  button->y = init.y;
  button->width = init.width;
  button->height = init.height;
  button->title = init.text;

  button->verticies = {
    button->x,                 button->y,
    button->x + button->width, button->y,
    button->x,                 button->y + button->height,
    button->x + button->width, button->y + button->height
  };
  button->indecies = {
    0, 1, 2,
    1, 3, 2
  };

  button->color_normal = init.color_normal;
  button->color_heighlight = init.color_heighlight;
  button->color_gray = init.color_gray;

  merlin::Shader shader = {};
  shader.name = "main";
  shader.vertex_path = "D:/projects/Empire/Empire/shaders/ui_button_vert.spv";
  shader.fragment_path = "D:/projects/Empire/Empire/shaders/ui_button_frag.spv";
  shader.geometry = false;

  merlin::Input_Binding binding;
  binding.binding = 0;
  binding.stride = sizeof(float)*5;
  binding.rate = merlin::RATE_VERTEX;

  merlin::Input_Attribute attribute_0;
  attribute_0.binding = 0;
  attribute_0.location = 0;
  attribute_0.loc_format = merlin::FVEC2;
  attribute_0.offset = 0;

  merlin::Input_Attribute attribute_1;
  attribute_1.binding = 0;
  attribute_1.location = 1;
  attribute_1.loc_format = merlin::FVEC3;
  attribute_1.offset = sizeof(float)*2;

  merlin::Input input = {};
  input.input_data = true;
  input.bindings = {binding};
  input.attributes = {attribute_0, attribute_1};

  merlin::Fixed_Functions fixed_functions = {};
  fixed_functions.reuse = false;
  fixed_functions.topology = merlin::TRIANGLE_LIST;
  fixed_functions.mode = merlin::FILL;
  fixed_functions.line_width = 1.0f;

  merlin::Multisampling multisampling = {};
  multisampling.enable = false;

  merlin::Uniform uniform = {};
  uniform.uniform = false;

  merlin::Attachment attachment = {};
  attachment.sample_count = 1;
  attachment.data_ops = merlin::LOAD_STORE;
  attachment.stencil_ops = merlin::DONT_CARE;
  attachment._layouts = merlin::PRESENT;

  merlin::Subpass subpass = {};
  subpass.point = merlin::GRAPHICS;
  subpass.color_attachments = {0};

  merlin::Render_Pass render_pass = {};
  render_pass.attachments = {attachment};
  render_pass.subpasses = {subpass};
  render_pass.dependencies = {};

  merlin::State_Init main_state_init = {};
  main_state_init.id = 1;
  main_state_init.shader = shader;
  main_state_init.input = input;
  main_state_init.fixed_functions = fixed_functions;
  main_state_init.multisampling = multisampling;
  main_state_init.uniform = uniform;
  main_state_init.render_pass = render_pass;

  merlin::Graph_Init graph_init = {};
  graph_init.activated = false;
  graph_init.loaded = {};
  graph_init.unloaded = {main_state_init};

  button->render_graph = merlin::create_graph(graph_init, window);
  return button;
}
void ui::destroy_button(Button* button) {
  merlin::destroy_graph(button->render_graph);
  delete button;
}

void ui::activate_graphics(Button* button) {
  merlin::load_state(1, &button->render_graph);
  merlin::activate_state(1, &button->render_graph);
}
void ui::deactivate_graphics(Button* button) {
  merlin::deactivate_state(&button->render_graph);
  merlin::unload_state(1, &button->render_graph);
}
bool ui::inside(float x, float y, Button* button) {
  bool x_check, y_check;
  if(x < button->x + button->width && x > button->x) {
    x_check = true;
  }
  else {
    x_check = false;
  }

  if(y < button->y + button->height && y > button->y) {
    y_check = true;
  }
  else {
    y_check = false;
  }

  return x_check && y_check;
}