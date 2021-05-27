#ifndef MERLIN_HELP
#define MERLIN_HELP

#include <vulkan/vulkan.h>
#include "../libs/SDL2/include/SDL.h"
#include <vector>

namespace merlin {
  namespace help {
    VkInstance create_instance(SDL_Window* window, bool debug);
    void find_indecies(uint32_t* present, uint32_t* graphics, uint32_t* transfer, VkSurfaceKHR surface, VkPhysicalDevice p_device);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos(uint32_t present, uint32_t graphics, uint32_t transfer);

    uint32_t get_image_count(VkSurfaceCapabilitiesKHR capabilities);
    VkSwapchainKHR create_swapchain(VkSurfaceCapabilitiesKHR capabilities, VkPresentModeKHR present_mode, uint32_t present_index, uint32_t graphics_index, VkExtent2D extent, VkSurfaceFormatKHR format, uint32_t image_count, VkSurfaceKHR surface, VkDevice device);
    VkSwapchainKHR recreate_swapchain(VkSwapchainKHR swapchain, VkSurfaceCapabilitiesKHR capabilities, VkPresentModeKHR present_mode, uint32_t present_index, uint32_t graphics_index, VkExtent2D extent, VkSurfaceFormatKHR format, uint32_t image_count, VkSurfaceKHR surface, VkDevice device);

    
  }
}

#endif
