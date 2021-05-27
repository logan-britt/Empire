#include "../include/merlin_draw.hpp"
#include "../include/merlin_help.hpp"

namespace merlin {
  Graph create_graph(Graph_Init init) {
    Graph graph = {};

    uint32_t loaded_count;
    if(init.activated) {
      loaded_count = init.loaded.size() + 1;
    }
    else {
      loaded_count = init.loaded.size();
    }
    graph.loaded_states.resize(loaded_count);
    graph.unloaded_states.resize(init.unloaded.size() + loaded_count);

    

    return graph;
  }
  void destroy_graph(Graph graph) {
    
  }

  void load_state(int id, Graph* graph) {

  }
  void unload_state(int id, Graph* graph) {

  }
  void activate_state(int id, Graph* graph) {

  }
}