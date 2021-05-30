#ifndef MERLIN
#define MERLIN

#ifdef _WIN32
  #define dllexport __declspec( dllexport )
#else 
  #define dllexport
#endif

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"

#include <vector>
#include <string>

enum gpu_type{DISCRETE, INTEGRATED};

namespace merlin {
  struct Engine;
  struct Window;


  void dllexport init();
  void dllexport terminate();

  struct dllexport Engine_Init
  {
    bool debug;
    gpu_type type;
  };
  struct dllexport Engine
  {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;

    uint32_t present_index;
    uint32_t graphics_index;
    uint32_t transfer_index;

    VkQueue present_queue;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    std::vector<Window*> linked_windows;
  };

  struct dllexport Window_Init
  {
    int width;
    int height;
    std::string title;
    bool resizable;
  };
  struct dllexport Window
  {
    int width;
    int height;
    std::string title;
    bool destroyed = false;

    SDL_Window* ptr;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSwapchainKHR old_swapchain;

    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkSurfaceCapabilitiesKHR capabilities;

    uint32_t image_count;
    VkExtent2D extent_2d;

    std::vector<VkImage> images;
    Engine* linked_engine;
  };
  
  void dllexport jump_engine(Window_Init win_init, Window* window, Engine_Init eng_init, Engine* engine);
  void dllexport destroy_engine(Engine engine);
  void dllexport destory_window(Window window);
}

#endif
