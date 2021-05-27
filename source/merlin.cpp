#include "../include/merlin.hpp"
#include "../include/merlin_help.hpp"

#include <cstdint>
#include <algorithm>
#include <iostream>
#include "../libs/SDL2/include/SDL_vulkan.h"

bool global_devices_inited = false;
std::vector<VkPhysicalDevice> global_devices = {};

uint32_t find_max_index(std::vector<int>* values) {
  int max_value = 0;
  int max_index = 0;
  int index = 0;
  std::vector<int>::iterator max_it;
  for(std::vector<int>::iterator it=values->begin(); it<values->end(); it++) {
    if(*it > max_value) {
      max_value = *it;
      max_index = index;
      max_it = it;
    }
    index += 1;
  }
  values->erase(max_it);
  return max_index;
}

VkPhysicalDevice borrow_device(gpu_type type) {
  int key_index = 0;
  bool device_found = false;
  for(uint32_t i=0; i<global_devices.size(); i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(global_devices[i], &props);

    switch(type)
    {
      case DISCRETE:
        if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
          key_index = i;
        }
        break;
      case INTEGRATED:
        if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
          key_index = i;
        }
        break;
    }
  }

  int search_index = 0;
  VkPhysicalDevice search_result;
  std::vector<VkPhysicalDevice>::iterator search_iterator;
  for(std::vector<VkPhysicalDevice>::iterator it=global_devices.begin(); it<global_devices.end(); it++) {
    if(key_index == search_index) {
      search_result = *it;
      search_iterator = it;
      break;
    }
    search_index += 1;
  }

  global_devices.erase(search_iterator);
  return search_result;
}

void return_device(VkPhysicalDevice physical_device) {
  global_devices.push_back(physical_device);
}

namespace merlin {
  void init() {
    if(SDL_Init( SDL_INIT_EVERYTHING ) != 0) {
      std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
      throw -1;
    }
  }
  void terminate() {
    SDL_Quit();
  }

  void jump_engine(Window_Init w_init, Window* window, Engine_Init e_init, Engine* engine) {
    window->width = w_init.width;
    window->height = w_init.height;
    window->title = w_init.title;

    if(!w_init.resizable) {
      window->ptr = SDL_CreateWindow(w_init.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w_init.width, w_init.height, SDL_WINDOW_VULKAN);
    }
    else {
      window->ptr = SDL_CreateWindow(w_init.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w_init.width, w_init.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    }
    engine->instance = help::create_instance(window->ptr, e_init.debug);

    if(!SDL_Vulkan_CreateSurface(window->ptr, engine->instance, &window->surface)) {
      std::cout << "Surface Creation Faild." << std::endl;
    }

    if(!global_devices_inited) {
      global_devices_inited = true;

      uint32_t physical_device_count;
      vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, nullptr);
      global_devices.resize(physical_device_count);
      vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, global_devices.data());
    }

    engine->physical_device = borrow_device(DISCRETE);
    help::find_indecies(&engine->present_index, &engine->graphics_index, &engine->transfer_index, window->surface, engine->physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_infos = help::create_queue_infos(engine->present_index, engine->graphics_index, engine->transfer_index);
    VkPhysicalDeviceFeatures features = {};
    features.fillModeNonSolid = true;
    features.geometryShader = true;
    features.tessellationShader = true;
    features.wideLines = true;
    std::vector<const char*> extensions = {
      VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = queue_infos.size();
    device_create_info.pQueueCreateInfos = queue_infos.data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = extensions.size();
    device_create_info.ppEnabledExtensionNames = extensions.data();
    device_create_info.enabledLayerCount = 0;

    VkResult res;
    res = vkCreateDevice(engine->physical_device, &device_create_info, nullptr, &engine->device);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      throw -3;
    }

    vkGetDeviceQueue(engine->device, engine->present_index, 0, &engine->present_queue);
    vkGetDeviceQueue(engine->device, engine->graphics_index, 0, &engine->graphics_queue);
    vkGetDeviceQueue(engine->device, engine->transfer_index, 0, &engine->transfer_queue);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physical_device, window->surface, &window->capabilities);

    uint32_t format_count;
    bool format_found = false;
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physical_device, window->surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physical_device, window->surface, &format_count, formats.data());
    for(auto test_format : formats) {
      bool format_check = test_format.format == VK_FORMAT_B8G8R8A8_SRGB;
      bool color_space_check = test_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      if(format_check && color_space_check) {
        window->format = test_format;
        format_found = true;
        break;
      }
    }
    if(!format_found) {
      window->format = formats[0];
    }

    uint32_t present_mode_count;
    bool present_mode_found = false;
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physical_device, window->surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physical_device, window->surface, &present_mode_count, present_modes.data());
    for(auto test_mode : present_modes) {
      bool present_mode_check = test_mode == VK_PRESENT_MODE_MAILBOX_KHR;
      if(present_mode_check) {
        window->present_mode = test_mode;
        present_mode_found = true;
        break;
      }
    }
    if(!present_mode_found) {
      window->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }

    if (window->capabilities.currentExtent.width != UINT32_MAX) {
      window->extent_2d = window->capabilities.currentExtent;
    }
    else {
      int width, height;
      SDL_Vulkan_GetDrawableSize(window->ptr, &width, &height);

      VkExtent2D actual_extent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height)
      };

      actual_extent.width = std::max(window->capabilities.minImageExtent.width, std::min(window->capabilities.maxImageExtent.width, actual_extent.width));
      actual_extent.height = std::max(window->capabilities.minImageExtent.height, std::min(window->capabilities.maxImageExtent.height, actual_extent.height));

      window->extent_2d = actual_extent;
    }

    window->image_count = help::get_image_count(window->capabilities);
    window->swapchain = help::create_swapchain(
      window->capabilities,
      window->present_mode,
      engine->present_index,
      engine->graphics_index,
      window->extent_2d,
      window->format,
      window->image_count,
      window->surface,
      engine->device
    );

    vkGetSwapchainImagesKHR(engine->device, window->swapchain, &window->image_count, nullptr);
    window->images.resize(window->image_count);
    vkGetSwapchainImagesKHR(engine->device, window->swapchain, &window->image_count, window->images.data());

    window->linked_engine = engine;
    engine->linked_windows.push_back(window);
  }
  void destroy_engine(Engine engine) {
    return_device(engine.physical_device);

    vkDestroyDevice(engine.device, nullptr);
    vkDestroyInstance(engine.instance, nullptr);
  }
  void destory_window(Window window) {
    if(!window.destroyed) {
      vkDestroySwapchainKHR(window.linked_engine->device, window.swapchain, nullptr);
      vkDestroySurfaceKHR(window.linked_engine->instance, window.surface, nullptr);
      SDL_DestroyWindow(window.ptr);
      window.destroyed = true;
    }
  }
}