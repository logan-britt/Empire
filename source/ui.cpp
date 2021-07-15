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
  input.bindings = { binding };
  input.attributes = {attribute_0, attribute_1};

  input.vertex_count = 4;
  input.instance_count = 1;
  input.index_count = button->indecies.size();

  merlin::Fixed_Functions fixed_functions = {};
  fixed_functions.reuse = false;
  fixed_functions.topology = merlin::TRIANGLE_LIST;
  fixed_functions.mode = merlin::FILL;
  fixed_functions.line_width = 1.0f;

  merlin::Multisampling multisampling = {};
  multisampling.enable = false;

  merlin::Uniform uniform = {};
  uniform.uniform = true;

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

  merlin::Buffer_Create_Info vertex_buffer_create_info = {};
  vertex_buffer_create_info.id = 1;
  vertex_buffer_create_info.raw = true;
  vertex_buffer_create_info.share = false;
  vertex_buffer_create_info.size = 20*sizeof(float);
  vertex_buffer_create_info.type = merlin::VERTEX;

  merlin::Buffer_Create_Info index_buffer_create_info = {};
  index_buffer_create_info.id = 2;
  index_buffer_create_info.raw = true;
  index_buffer_create_info.share = false;
  index_buffer_create_info.size = 6*sizeof(uint32_t);
  index_buffer_create_info.type = merlin::INDEX;

  merlin::Buffer_Info buffer_info = {};
  buffer_info.copy_infos = {};
  buffer_info.create_infos = { vertex_buffer_create_info, index_buffer_create_info };

  merlin::State_Init main_state_init = {};
  main_state_init.id = 1;
  main_state_init.shader = shader;
  main_state_init.input = input;
  main_state_init.fixed_functions = fixed_functions;
  main_state_init.multisampling = multisampling;
  main_state_init.uniform = uniform;
  main_state_init.render_pass = render_pass;
  main_state_init.buffer_info = buffer_info;

  merlin::State_Init high_state_init = {};
  high_state_init.id = 2;
  high_state_init.shader = shader;
  high_state_init.input = input;
  high_state_init.fixed_functions = fixed_functions;
  high_state_init.multisampling = multisampling;
  high_state_init.uniform = uniform;
  high_state_init.render_pass = render_pass;
  high_state_init.buffer_info = buffer_info;

  merlin::State_Init gray_state_init = {};
  gray_state_init.id = 3;
  gray_state_init.shader = shader;
  gray_state_init.input = input;
  gray_state_init.fixed_functions = fixed_functions;
  gray_state_init.multisampling = multisampling;
  gray_state_init.uniform = uniform;
  gray_state_init.render_pass = render_pass;
  gray_state_init.buffer_info = buffer_info;

  merlin::Graph_Init graph_init = {};
  graph_init.id = 1;
  graph_init.pool = init.pool;
  graph_init.allocator = init.allocator;

  graph_init.activated = true;
  graph_init.active = main_state_init;
  graph_init.loaded = { high_state_init, gray_state_init };
  graph_init.unloaded = {};
  
  button->render_graph = merlin::create_graph(graph_init, window);

  std::vector<float> normal_data(4*5, 0.0f);
  for(uint32_t i=0; i<4; i++) {
    normal_data[5*i]     = button->verticies[2*i];
    normal_data[5*i + 1] = button->verticies[2*i + 1];
    normal_data[5*i + 2] = button->color_normal[0];
    normal_data[5*i + 3] = button->color_normal[1];
    normal_data[5*i + 4] = button->color_normal[2];
  }
  fill_state_with_floats(normal_data, 1, 1, &button->render_graph);
  fill_state_with_ints(button->indecies, 1, 2, &button->render_graph);

  std::vector<float> highlight_data(4*5, 0.0f);
  for(uint32_t i=0; i<4; i++) {
    highlight_data[5*i]     = button->verticies[2*i];
    highlight_data[5*i + 1] = button->verticies[2*i + 1];
    highlight_data[5*i + 2] = button->color_heighlight[0];
    highlight_data[5*i + 3] = button->color_heighlight[1];
    highlight_data[5*i + 4] = button->color_heighlight[2];
  }
  fill_state_with_floats(highlight_data, 2, 1, &button->render_graph);
  fill_state_with_ints(button->indecies, 2, 2, &button->render_graph);

  std::vector<float> gray_data(4*5, 0.0f);
  for(uint32_t i=0; i<4; i++) {
    gray_data[5*i]     = button->verticies[2*i];
    gray_data[5*i + 1] = button->verticies[2*i + 1];
    gray_data[5*i + 2] = button->color_gray[0];
    gray_data[5*i + 3] = button->color_gray[1];
    gray_data[5*i + 4] = button->color_gray[2];
  }
  fill_state_with_floats(gray_data, 3, 1, &button->render_graph);
  fill_state_with_ints(button->indecies, 3, 2, &button->render_graph);

  return button;
}
void ui::destroy_button(Button* button) {
  merlin::destroy_graph(button->render_graph);
  delete button;
}

void ui::set_to_normal(Button* button) {
  merlin::activate_state(1, &button->render_graph);
}
void ui::set_to_hightlight(Button* button) {
  merlin::activate_state(2, &button->render_graph);
}
void ui::set_to_gray(Button* button) {
  merlin::activate_state(3, &button->render_graph);
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