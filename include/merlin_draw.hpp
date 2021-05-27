#ifndef MERLIN_GRAPH
#define MERLIN_GRAPH

#define dllexport __declspec( dllexport )

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"
#include "../include/merlin.hpp"

#include <vector>
#include <string>

namespace merlin {
  struct dllexport Shader
  {
    std::string name;
    std::string vertex_path;
    std::string fragment_path;
    bool geometry;
    std::string geometry_path;
  };
  struct dllexport Fixed_Functions
  {
  };
  struct dllexport State_Init
  {
    int id;
    Shader shader;
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
    State* active_state;
    std::vector<State> loaded_states;
    std::vector<State_Init> unloaded_states;
  };

  Graph dllexport create_graph(Graph_Init init);
  void dllexport destroy_graph(Graph graph);

  void dllexport load_state(int id, Graph* graph);
  void dllexport unload_state(int id, Graph* graph);
  void dllexport activate_state(int id, Graph* graph);
}

#endif