#ifndef MERLIN_GRAPH
#define MERLIN_GRAPH

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"

#include "../include/vk_mem_alloc.h"
#include "../include/merlin.hpp"

#include <vector>
#include <string>

namespace merlin {
  enum input_topology{POINT_LIST, LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP, TRIANGLE_FAN};
  enum polygon_mode{FILL, LINE, POINT};
  enum ops{LOAD_STORE, LOAD_DONT_CARE, CLEAR_STORE, CLEAR_DONT_CARE, DONT_CARE_STORE, DONT_CARE};
  enum layouts{UNDEFINED_PRESENT, PRESENT};
  enum bind_point{GRAPHICS};
  enum input_rate{RATE_VERTEX, RATE_INTANCE};
  enum input_type{FLOAT, DOUBLE};
  enum loacation_format{FVEC1, FVEC2, FVEC3, FVEC4, DVEC1, DVEC2, DVEC3, DVEC4};

  struct Shader
  {
    std::string name;
    std::string vertex_path;
    std::string fragment_path;
    bool geometry;
    std::string geometry_path;
  };

  struct Input_Binding
  {
    int binding;
    size_t stride;
    input_rate rate;
  };
  struct Input_Attribute
  {
    int binding;
    int location;
    loacation_format loc_format;
    size_t offset;
  };
  struct Input
  {
    bool input_data;
    std::vector<Input_Binding> bindings;
    std::vector<Input_Attribute> attributes;

    size_t vertex_count;
    size_t instance_count;
  };

  struct Fixed_Functions
  {
    bool reuse;
    input_topology topology;
    polygon_mode mode;
    float line_width;
  };
  struct Multisampling
  {
    bool enable;
  };

  struct Uniform
  {
    bool uniform;
  };

  struct Attachment
  {
    int sample_count;
    ops data_ops;
    ops stencil_ops;
    layouts _layouts;
  };
  struct Subpass
  {
    bind_point point;
    bool depth_stencil;
    int depth_stencil_attachment;
    std::vector<int> color_attachments;
    std::vector<int> input_attachments;
    std::vector<int> preserve_attachments;
    std::vector<int> resolve_attachments;
  };
  struct Dependency
  {
  };
  struct Render_Pass
  {
    std::vector<Attachment> attachments;
    std::vector<Subpass> subpasses;
    std::vector<Dependency> dependencies;
  };

  struct State_Init
  {
    int id;
    Shader shader;
    Input input;
    Fixed_Functions fixed_functions;
    Multisampling multisampling;
    Uniform uniform;
    Render_Pass render_pass;
  };
  struct State
  {
    int id;

    VkShaderModule vertex_module;
    VkShaderModule fragment_module;
    bool geometry;
    VkShaderModule geometry_module;

    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;

    VkBuffer transfer_buffer;
    std::vector<VkBuffer> vertex_buffers;
    std::vector<VkBuffer> index_buffers;
    std::vector<VkBuffer> uniform_buffers;

    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> draw_buffers;
  };

  struct Graph_Init
  {
    int id;
    bool activated = false;
    State_Init active;
    std::vector<State_Init> loaded;
    std::vector<State_Init> unloaded;
  };

  struct Graph
  {
    int id;

    std::vector<VkImageView> views;
    VkCommandPool draw_pool;
    VkCommandPool transfer_pool;

    bool activated;
    State* active_state;
    std::vector<State> loaded_states;
    std::vector<State_Init> unloaded_states;

    bool linked;
    Window* linked_window;
  };

  Graph create_graph(Graph_Init init, Window* window);
  void destroy_graph(Graph graph);

  void load_state(int id, Graph* graph);
  void unload_state(int id, Graph* graph);
  void activate_state(int id, Graph* graph);
  void deactivate_state(Graph* graph);

  void draw(std::vector<Graph*> graphs);
}

#endif