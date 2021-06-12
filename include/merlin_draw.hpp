#ifndef MERLIN_GRAPH
#define MERLIN_GRAPH

#ifdef _WIN32
  #define dllexport __declspec( dllexport )
#else
  #define dllexport
#endif

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"
#include "../include/merlin.hpp"

#include <vector>
#include <string>

namespace merlin {
  enum input_topology{POINT_LIST, LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP, TRIANGLE_FAN};
  enum polygon_mode{FILL, LINE, POINT};
  enum ops{LOAD_STORE, LOAD_DONT_CARE, CLEAR_STORE, CLEAR_DONT_CARE, DONT_CARE_STORE, DONT_CARE};
  enum layouts{UNDEFINED_PRESENT};
  enum bind_point{GRAPHICS};

  struct dllexport Shader
  {
    std::string name;
    std::string vertex_path;
    std::string fragment_path;
    bool geometry;
    std::string geometry_path;
  };
  struct dllexport Input
  {
    bool input_data;
  };
  struct dllexport Fixed_Functions
  {
    bool reuse;
    input_topology topology;
    polygon_mode mode;
    float line_width;
  };
  struct dllexport Multisampling
  {
    bool enable;
  };
  struct dllexport Uniform
  {
    bool uniform;
  };

  struct dllexport Attachment
  {
    int sample_count;
    ops data_ops;
    ops stencil_ops;
    layouts _layouts;
  };
  struct dllexport Subpass
  {
    bind_point point;
    bool depth_stencil;
    int depth_stencil_attachment;
    std::vector<int> color_attachments;
    std::vector<int> input_attachments;
    std::vector<int> preserve_attachments;
    std::vector<int> resolve_attachments;
  };
  struct dllexport Dependency
  {
  };
  struct dllexport Render_Pass
  {
    std::vector<Attachment> attachments;
    std::vector<Subpass> subpasses;
    std::vector<Dependency> dependencies;

  };

  struct dllexport State_Init
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

    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> draw_buffers;
  };


  struct dllexport Graph_Init
  {
    bool activated = false;
    State_Init active;
    std::vector<State_Init> loaded;
    std::vector<State_Init> unloaded;
  };

  struct dllexport Graph
  {
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

  Graph dllexport create_graph(Graph_Init init, Window* window);
  void dllexport destroy_graph(Graph graph);

  void dllexport load_state(int id, Graph* graph);
  void dllexport unload_state(int id, Graph* graph);
  void dllexport activate_state(int id, Graph* graph);
  void dllexport deactivate_state(Graph* graph);

  void dllexport draw(std::vector<Graph*> graphs);
}

#endif