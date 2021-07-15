#ifndef MERLIN
#define MERLIN

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"
#include "../libs/glm/glm/mat2x2.hpp"

#include <vector>
#include <string>

enum gpu_type{DISCRETE, INTEGRATED};

namespace merlin {
  struct Engine;
  struct Window;


  void init();
  void terminate();

  struct Engine_Init
  {
    bool debug;
    gpu_type type;
  };
  struct Engine
  {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;

    bool debug;
    VkDebugUtilsMessengerEXT debug_messenger;

    uint32_t present_index;
    uint32_t graphics_index;
    uint32_t transfer_index;

    VkQueue present_queue;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    std::vector<Window*> linked_windows;
  };

  struct Window_Init
  {
    int width;
    int height;
    std::string title;
    bool resizable;
    float r, g, b;
  };
  struct Window
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

    uint32_t max_frames;
    uint32_t current_frame;

    VkClearColorValue clear_color;
    VkImageSubresourceRange image_range;
    std::vector<VkFence> clear_fences;

    glm::mat2x2 transform;

    VkCommandPool clear_pool;
    std::vector<VkCommandBuffer> clear_command_buffers;

    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;

    std::vector<VkImage> images;
    Engine* linked_engine;
  };
  
  void jump_engine(Window_Init win_init, Window* window, Engine_Init eng_init, Engine* engine);
  void destroy_engine(Engine engine);
  void destory_window(Window window);
}

#endif
