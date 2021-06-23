#include "../include/merlin_help.hpp"

#include "../libs/SDL2/include/SDL_vulkan.h"

#include <iostream>
#include <vector>
#include <fstream>

uint32_t find_minimum_index(std::vector<int> values) {
  int min = 100;
  uint32_t min_index = 0;
  for(uint32_t i=0; i<values.size(); i++) {
    if(min > values[i]) {
      min = values[i];
      min_index = i;
    }
  }
  return min_index;
}

std::vector<char> read_file(std::string path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if(!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t) file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

namespace merlin {
  namespace help {
    VkInstance create_instance(SDL_Window* window, bool debug) {
      VkResult res;

      VkApplicationInfo application_info = {};
      application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      application_info.pNext = nullptr;
      application_info.apiVersion = VK_API_VERSION_1_2;
    
      VkInstanceCreateInfo instance_create_info = {};
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pNext = nullptr;
      instance_create_info.flags = 0;
      instance_create_info.pApplicationInfo = &application_info;

      std::vector<const char*> layer_names = {};
      if(debug) {
        layer_names = {"VK_LAYER_KHRONOS_validation"};
      }
      else {
        layer_names = {};
      }
      instance_create_info.enabledLayerCount = layer_names.size();
      instance_create_info.ppEnabledLayerNames = layer_names.data();

      uint32_t extension_count;
      SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
      std::vector<const char*> extension_names(extension_count);
      SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names.data());
      if(debug) {
        extension_count += 1;
        extension_names.push_back("VK_EXT_debug_utils");
      }

      instance_create_info.enabledExtensionCount = extension_count;
      instance_create_info.ppEnabledExtensionNames = extension_names.data();

      VkInstance instance;
      res = vkCreateInstance(&instance_create_info, nullptr, &instance);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "Instance creation faild. Shutting Down." << std::endl;
        throw;
      }

      return instance;
    }

    void find_indecies(uint32_t* present, uint32_t* graphics, uint32_t* transfer, VkSurfaceKHR surface, VkPhysicalDevice p_device) {
      uint32_t family_count;
      vkGetPhysicalDeviceQueueFamilyProperties(p_device, &family_count, nullptr);
      std::vector<VkQueueFamilyProperties> family_props(family_count);
      vkGetPhysicalDeviceQueueFamilyProperties(p_device, &family_count, family_props.data());
      
      bool combined_graphics = false;
      std::vector<int> present_scores(family_count);
      std::vector<int> graphics_scores(family_count);
      std::vector<int> transfer_scores(family_count);
      for(uint32_t i=0; i<family_count; i++) {
        bool graphics_check = family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        bool transfer_check = family_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
        VkBool32 present_check;
        vkGetPhysicalDeviceSurfaceSupportKHR(p_device, i, surface, &present_check);

        if(present_check && graphics_check && !combined_graphics) {
          combined_graphics = true;
          *present = i;
          *graphics = i;
        }

        int score = 0;
        if(present_check) {
          score += 1;
        }
        if(graphics_check) {
          score += 1;
        }
        if(transfer_check) {
          score += 1;
        }

        int max_score = 10;
        if(present_check) {
          present_scores[i] = score;
        }
        else {
          present_scores[i] = max_score;
        }
        if(graphics_check) {
          graphics_scores[i] = score;
        }
        else {
          graphics_scores[i] = max_score;
        }
        if(transfer_check) {
          transfer_scores[i] = score;
        }
        else {
          transfer_scores[i] = max_score;
        }
      }

      if(!combined_graphics) {
        *present = find_minimum_index(present_scores);
        *graphics = find_minimum_index(graphics_scores);
      }
      *transfer = find_minimum_index(transfer_scores);
    }

    std::vector<VkDeviceQueueCreateInfo> create_queue_infos(uint32_t present, uint32_t graphics, uint32_t transfer) {
      float priorities = 1.0f;
      std::vector<VkDeviceQueueCreateInfo> queue_infos = {};

      if(present == graphics && present == transfer) {
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.pQueuePriorities = &priorities;
        info.queueCount = 1;
        info.queueFamilyIndex = present;

        queue_infos = {info};
      }
      else if(present == graphics) {
        VkDeviceQueueCreateInfo shared_info = {};
        shared_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        shared_info.pNext = nullptr;
        shared_info.flags = 0;
        shared_info.pQueuePriorities = &priorities;
        shared_info.queueCount = 1;
        shared_info.queueFamilyIndex = present;

        VkDeviceQueueCreateInfo transfer_info = {};
        transfer_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transfer_info.pNext = nullptr;
        transfer_info.flags = 0;
        transfer_info.pQueuePriorities = &priorities;
        transfer_info.queueCount = 1;
        transfer_info.queueFamilyIndex = transfer;

        queue_infos = {shared_info, transfer_info};
      }
      else if(present == transfer) {
        VkDeviceQueueCreateInfo shared_info = {};
        shared_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        shared_info.pNext = nullptr;
        shared_info.flags = 0;
        shared_info.pQueuePriorities = &priorities;
        shared_info.queueCount = 1;
        shared_info.queueFamilyIndex = present;

        VkDeviceQueueCreateInfo graphics_info = {};
        graphics_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_info.pNext = nullptr;
        graphics_info.flags = 0;
        graphics_info.pQueuePriorities = &priorities;
        graphics_info.queueCount = 1;
        graphics_info.queueFamilyIndex = graphics;

        queue_infos = {shared_info, graphics_info};
      }
      else if(graphics == transfer) {
        VkDeviceQueueCreateInfo shared_info = {};
        shared_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        shared_info.pNext = nullptr;
        shared_info.flags = 0;
        shared_info.pQueuePriorities = &priorities;
        shared_info.queueCount = 1;
        shared_info.queueFamilyIndex = graphics;

        VkDeviceQueueCreateInfo present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_info.pNext = nullptr;
        present_info.flags = 0;
        present_info.pQueuePriorities = &priorities;
        present_info.queueCount = 1;
        present_info.queueFamilyIndex = present;

        queue_infos = {shared_info, present_info};
      }
      else {
        VkDeviceQueueCreateInfo present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_info.pNext = nullptr;
        present_info.flags = 0;
        present_info.pQueuePriorities = &priorities;
        present_info.queueCount = 1;
        present_info.queueFamilyIndex = present;

        VkDeviceQueueCreateInfo graphics_info = {};
        graphics_info.pNext = nullptr;
        graphics_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_info.flags = 0;
        graphics_info.pQueuePriorities = &priorities;
        graphics_info.queueCount = 1;
        graphics_info.queueFamilyIndex = graphics;

        VkDeviceQueueCreateInfo transfer_info = {};
        transfer_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transfer_info.pNext = nullptr;
        transfer_info.flags = 0;
        transfer_info.pQueuePriorities = &priorities;
        transfer_info.queueCount = 1;
        transfer_info.queueFamilyIndex = transfer;

        queue_infos = {present_info, graphics_info, transfer_info};
      }
      return queue_infos;
    }

    uint32_t get_image_count(VkSurfaceCapabilitiesKHR capabilities) {
      uint32_t image_count = capabilities.minImageCount + 1;
      if(capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
      }
      return image_count;
    }
    VkSwapchainKHR create_swapchain(VkSurfaceCapabilitiesKHR capabilities, VkPresentModeKHR present_mode, uint32_t present_index, uint32_t graphics_index, VkExtent2D extent, VkSurfaceFormatKHR format, uint32_t image_count, VkSurfaceKHR surface, VkDevice device) {
      VkSwapchainCreateInfoKHR swapchain_create_info = {};
      swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      swapchain_create_info.pNext = nullptr;
      swapchain_create_info.flags = 0;
      swapchain_create_info.surface = surface;
      swapchain_create_info.minImageCount = image_count;
      swapchain_create_info.imageFormat = format.format;
      swapchain_create_info.imageColorSpace = format.colorSpace;
      swapchain_create_info.imageExtent = extent;
      swapchain_create_info.imageArrayLayers = 1;
      swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      
      uint32_t queueFamilyIndices[] = {present_index, graphics_index};
      if(present_index != graphics_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
      }
      else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
      }

      swapchain_create_info.preTransform = capabilities.currentTransform;
      swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      swapchain_create_info.presentMode = present_mode;
      swapchain_create_info.clipped = VK_TRUE;
      swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

      VkResult res;
      VkSwapchainKHR swapchain;
      res = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The swapchain creation faild. Shutting Down." << std::endl;
        throw;
      }
      return swapchain;
    }
    VkSwapchainKHR recreate_swapchain(VkSwapchainKHR swapchain, VkSurfaceCapabilitiesKHR capabilities, VkPresentModeKHR present_mode, uint32_t present_index, uint32_t graphics_index, VkExtent2D extent, VkSurfaceFormatKHR format, uint32_t image_count, VkSurfaceKHR surface, VkDevice device) {
      VkSwapchainCreateInfoKHR swapchain_create_info = {};
      swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      swapchain_create_info.pNext = nullptr;
      swapchain_create_info.flags = 0;
      swapchain_create_info.surface = surface;
      swapchain_create_info.minImageCount = image_count;
      swapchain_create_info.imageFormat = format.format;
      swapchain_create_info.imageColorSpace = format.colorSpace;
      swapchain_create_info.imageExtent = extent;
      swapchain_create_info.imageArrayLayers = 1;
      swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      
      uint32_t queueFamilyIndices[] = {present_index, graphics_index};
      if(present_index != graphics_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
      }
      else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
      }

      swapchain_create_info.preTransform = capabilities.currentTransform;
      swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      swapchain_create_info.presentMode = present_mode;
      swapchain_create_info.clipped = VK_TRUE;
      swapchain_create_info.oldSwapchain = swapchain;

      VkResult res;
      VkSwapchainKHR new_swapchain;
      res = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The swapchain recreation faild. Shutting Down." << std::endl;
        throw;
      }
      return new_swapchain;
    }

    VkShaderModule create_shader_module(std::string shader_path, VkDevice device) {
      std::vector<char> code = read_file(shader_path);

      VkShaderModuleCreateInfo module_create_info = {};
      module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      module_create_info.pNext = nullptr;
      module_create_info.flags = 0;
      module_create_info.codeSize = code.size();
      module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

      VkShaderModule shader_module;
      VkResult res = vkCreateShaderModule(device, &module_create_info, nullptr, &shader_module);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "Shader module could not be created. Shutting down." << std::endl;
        throw;
      }
      return shader_module;
    }

    VkPipelineInputAssemblyStateCreateInfo create_input_assemply(input_topology topology, bool reuse) {
      VkPipelineInputAssemblyStateCreateInfo input_assemply = {};
      input_assemply.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      input_assemply.pNext = nullptr;
      input_assemply.flags = 0;
      switch(topology)
      {
        case POINT_LIST:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
          break;
        case LINE_LIST:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
          break;
        case LINE_STRIP:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
          break;
        case TRIANGLE_LIST:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
          break;
        case TRIANGLE_STRIP:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
          break;
        case TRIANGLE_FAN:
          input_assemply.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
          break;
      }
      if(reuse) {
        input_assemply.primitiveRestartEnable = VK_TRUE;
      }
      else {
        input_assemply.primitiveRestartEnable = VK_FALSE;
      }

      return input_assemply;
    }

    VkAttachmentDescription create_attachment_description(int sample_count, VkFormat image_format, ops data_ops, ops stencil_ops, layouts layouts) {
      VkAttachmentDescription attachment = {};
      attachment.flags = 0;
      attachment.format = image_format;
      switch(sample_count)
      {
        case 1:
          attachment.samples = VK_SAMPLE_COUNT_1_BIT;
          break;
        case 2:
          attachment.samples = VK_SAMPLE_COUNT_2_BIT;
          break;
        case 4:
          attachment.samples = VK_SAMPLE_COUNT_4_BIT;
          break;
        case 8:
          attachment.samples = VK_SAMPLE_COUNT_8_BIT;
          break;
        case 16:
          attachment.samples = VK_SAMPLE_COUNT_16_BIT;
          break;
        case 32:
          attachment.samples = VK_SAMPLE_COUNT_32_BIT;
          break;
        case 64:
          attachment.samples = VK_SAMPLE_COUNT_64_BIT;
          break;
      }
      switch(data_ops)
      {
        case LOAD_STORE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case LOAD_DONT_CARE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
        case CLEAR_STORE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case CLEAR_DONT_CARE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
        case DONT_CARE_STORE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case DONT_CARE:
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
      }
      switch(stencil_ops)
      {
        case LOAD_STORE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case LOAD_DONT_CARE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
        case CLEAR_STORE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case CLEAR_DONT_CARE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
        case DONT_CARE_STORE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
          break;
        case DONT_CARE:
          attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          break;
      }
      switch(layouts)
      {
        case UNDEFINED_PRESENT:
          attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
          attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          break;
        case PRESENT:
          attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          break;
      }
      return attachment;
    }

    VkImageLayout choose_layout(layouts transition_layouts) {
      VkImageLayout layout;
      switch(transition_layouts)
      {
        case UNDEFINED_PRESENT:
          layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
          break;
      }
      return layout;
    }
  }
}
