#include "../include/ui.hpp"

ui::Button* ui::create_button(Button_Init init, merlin::Window* window) {
  Button* button = new Button;
  button->x = init.x;
  button->y = init.y;
  button->width = init.width;
  button->height = init.height;
  button->title = init.text;

  merlin::Shader shader = {};
  shader.name = "main";
  shader.vertex_path = "D:/projects/Empire/Empire/shaders/ui_button_vert.spv";
  shader.fragment_path = "D:/projects/Empire/Empire/shaders/ui_button_frag.spv";
  shader.geometry = false;

  merlin::Input input = {};
  input.input_data = false;

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
  attachment.data_ops = merlin::CLEAR_STORE;
  attachment.stencil_ops = merlin::DONT_CARE;
  attachment._layouts = merlin::UNDEFINED_PRESENT;

  merlin::Subpass subpass = {};
  subpass.point = merlin::GRAPHICS;
  subpass.color_attachments = {0};

  merlin::Render_Pass render_pass = {};
  render_pass.attachments = {attachment};
  render_pass.subpasses = {subpass};
  render_pass.dependencies = {};

  merlin::State_Init state_init = {};
  state_init.id = 1;
  state_init.shader = shader;
  state_init.input = input;
  state_init.fixed_functions = fixed_functions;
  state_init.multisampling = multisampling;
  state_init.uniform = uniform;
  state_init.render_pass = render_pass;

  merlin::Graph_Init graph_init = {};
  graph_init.activated = false;
  graph_init.loaded = {};
  graph_init.unloaded = {state_init};

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