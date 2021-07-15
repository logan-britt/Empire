#include "../include/merlin.hpp"
#include "../include/merlin_help.hpp"

#include <cstdint>
#include <iostream>
#include <algorithm>
#include "../libs/SDL2/include/SDL_vulkan.h"

VkResult create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if(func != nullptr){
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else{
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
  //std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
  //std::cout << std::endl;
  return VK_FALSE;
}

namespace merlin {
  void init() {
    if(SDL_Init( SDL_INIT_EVERYTHING ) != 0) {
      std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
      throw;
    }
  }
  void terminate() {
    SDL_Quit();
  }

  void jump_engine(Window_Init w_init, Window* window, Engine_Init e_init, Engine* engine) {
    VkResult res;

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

    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};
    debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_create_info.pNext = nullptr;
    debug_messenger_create_info.flags = 0;
    debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_messenger_create_info.messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_messenger_create_info.pfnUserCallback = debug_callback;
    debug_messenger_create_info.pUserData = nullptr;

    engine->debug = e_init.debug;
    if(e_init.debug) {
      res = create_debug_utils_messenger_EXT(engine->instance, &debug_messenger_create_info, nullptr, &engine->debug_messenger);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The debugger could not be created. Shutting down." << std::endl;
        throw;
      }
    }

    if(!SDL_Vulkan_CreateSurface(window->ptr, engine->instance, &window->surface)) {
      std::cerr << "Surface Creation Faild." << std::endl;
      throw;
    }

    uint32_t physical_device_count;
    bool physical_device_found = false;
    vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(engine->instance, &physical_device_count, physical_devices.data());
    for(uint32_t i=0; i<physical_device_count; i++) {
      VkPhysicalDeviceProperties device_props;
      vkGetPhysicalDeviceProperties(physical_devices[i], &device_props);

      if(e_init.type == DISCRETE && device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        engine->physical_device = physical_devices[i];
        physical_device_found = true;
        break;
      }
      else if(e_init.type == INTEGRATED && device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        engine->physical_device = physical_devices[i];
        physical_device_found = true;
        break;
      }
    }
    if(!physical_device_found) {
      engine->physical_device = physical_devices[0];
    }

    help::find_indecies(&engine->present_index, &engine->graphics_index, &engine->transfer_index, window->surface, engine->physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_infos = help::create_queue_infos(engine->present_index, engine->graphics_index, engine->transfer_index);
    
    VkPhysicalDeviceFeatures features = {};
    features.fillModeNonSolid = true;
    features.geometryShader = true;
    features.tessellationShader = true;
    features.wideLines = true;
    
    std::vector<const char*> device_extensions = {
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
    device_create_info.enabledExtensionCount = device_extensions.size();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledLayerCount = 0;

    res = vkCreateDevice(engine->physical_device, &device_create_info, nullptr, &engine->device);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "Device creation faild. Shuttin Down." << std::endl;
      throw;
    }

    vkGetDeviceQueue(engine->device, engine->present_index, 0, &engine->present_queue);
    vkGetDeviceQueue(engine->device, engine->graphics_index, 0, &engine->graphics_queue);
    vkGetDeviceQueue(engine->device, engine->transfer_index, 0, &engine->transfer_queue);

    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physical_device, window->surface, &window->capabilities);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "We could not get surface capabilities. Shutting down." << std::endl;
      throw;
    }

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

    window->max_frames = 3;
    window->current_frame = 0;

    VkCommandPoolCreateInfo clear_pool_create_info = {};
    clear_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    clear_pool_create_info.pNext = nullptr;
    clear_pool_create_info.flags = 0;
    clear_pool_create_info.queueFamilyIndex = engine->graphics_index;

    res = vkCreateCommandPool(engine->device, &clear_pool_create_info, nullptr, &window->clear_pool);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The command pool could not be created. Shutting down." << std::endl;
      throw;
    }

    window->clear_command_buffers.resize(window->image_count);

    VkCommandBufferAllocateInfo command_buffer_allocation_info{};
    command_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocation_info.commandPool = window->clear_pool;
    command_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocation_info.commandBufferCount = (uint32_t) window->clear_command_buffers.size();

    res = vkAllocateCommandBuffers(engine->device, &command_buffer_allocation_info, window->clear_command_buffers.data());
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The command buffer for clear opperation could not be allocated. Shutting down." << std::endl;
      throw;
    }

    for(uint32_t i=0; i<window->clear_command_buffers.size(); i++) {
      VkCommandBufferBeginInfo command_buffer_begin_info = {};
      command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      command_buffer_begin_info.pNext = nullptr;
      command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      command_buffer_begin_info.pInheritanceInfo = nullptr;

      res = vkBeginCommandBuffer(window->clear_command_buffers[i], &command_buffer_begin_info);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The command buffer for the clear operation could not be began. Shutting down." << std::endl;
        throw;
      }

      window->clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

      window->image_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      window->image_range.levelCount = 1;
      window->image_range.layerCount = 1;

      vkCmdClearColorImage(window->clear_command_buffers[i], window->images[i], VK_IMAGE_LAYOUT_GENERAL, &window->clear_color, 1, &window->image_range);

      res = vkEndCommandBuffer(window->clear_command_buffers[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The command buffer for the clear opperation could not be ended. Shutting down." << std::endl;
        throw;
      }
    }

    VkFenceCreateInfo clear_fence_create_info = {};
    clear_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    window->clear_fences.resize(window->image_count);
    for(uint32_t i=0; i<window->image_count; i++) {
      res = vkCreateFence(engine->device, &clear_fence_create_info, nullptr, &window->clear_fences[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The fences for the clear operation could not be created. Shutting down." << std::endl;
        throw;
      }
    }

    VkSemaphoreTypeCreateInfoKHR binary_info = {};
    binary_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    binary_info.pNext = nullptr;
    binary_info.initialValue = 0;
    binary_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;

    VkSemaphoreCreateInfo binary_create_info = {};
    binary_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    binary_create_info.pNext = &binary_info;
    binary_create_info.flags = 0;

    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = nullptr;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    window->in_flight_fences.resize(window->max_frames);
    window->images_in_flight.resize(window->image_count, VK_NULL_HANDLE);
    window->render_finished_semaphores.resize(window->max_frames);
    window->image_available_semaphores.resize(window->max_frames);
    for(uint32_t i=0; i<window->max_frames; i++) {
      res = vkCreateSemaphore(engine->device, &binary_create_info, nullptr, &window->image_available_semaphores[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The semaphore could not be created. Shutting Down." << std::endl;
        throw;
      }

      res = vkCreateSemaphore(engine->device, &binary_create_info, nullptr, &window->render_finished_semaphores[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The semaphore could not be created. Shutting down." << std::endl;
        throw;
      }

      res = vkCreateFence(engine->device, &fence_create_info, nullptr, &window->in_flight_fences[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The fence could not be created. Shutting down." << std::endl;
        throw;
      }
    }

    glm::vec2 collumn_0 = {2.0f/(float)window->width, 0.0f};
    glm::vec2 collumn_1 = {0.0f, -2.0f/(float)window->height};
    window->transform = {collumn_0, collumn_1};

    window->linked_engine = engine;
    engine->linked_windows = {window};
  }
  void destroy_engine(Engine engine) {
    while(!engine.linked_windows.empty()) {
      auto window_it = engine.linked_windows.begin();
      auto window_ptr = *window_it;
      
      destory_window(*window_ptr);
      engine.linked_windows.erase(window_it);
    }

    vkDestroyDevice(engine.device, nullptr);
    if(engine.debug) {
      destroy_debug_utils_messenger_EXT(engine.instance, engine.debug_messenger, nullptr);
    }
    vkDestroyInstance(engine.instance, nullptr);
  }
  void destory_window(Window window) {
    if(!window.destroyed) {
      vkDeviceWaitIdle(window.linked_engine->device);
      for(uint32_t i=0; i<window.max_frames; i++) {
        vkDestroySemaphore(window.linked_engine->device, window.image_available_semaphores[i], nullptr);
        vkDestroySemaphore(window.linked_engine->device, window.render_finished_semaphores[i], nullptr);

        vkDestroyFence(window.linked_engine->device, window.in_flight_fences[i], nullptr);
      }

      vkDestroySwapchainKHR(window.linked_engine->device, window.swapchain, nullptr);
      vkDestroySurfaceKHR(window.linked_engine->instance, window.surface, nullptr);
      SDL_DestroyWindow(window.ptr);
      window.destroyed = true;
    }
  }
}