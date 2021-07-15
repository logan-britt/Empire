#include "../include/merlin_memory.hpp"

namespace merlin
{
  memory_size BYTE = 1;
  memory_size KILOBYTE = 1000;
  memory_size MEGABYTE = 1000000;
  memory_size GIGABYTE = 1000000000;

  buffer_type VERTEX = 1;
  buffer_type INDEX = 2;
  buffer_type UNIFORM = 4;
  buffer_type TRANSFER = 8;

  memory_type HOST_VISIBLE = 1;
  memory_type HOST_COHERENT = 2;
  memory_type DEVICE_LOCAL = 4;

  uint32_t find_memory_index(memory_type type, VkPhysicalDevice physical_device) {
    uint32_t index;

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    for(uint32_t i=0; i<memory_properties.memoryTypeCount; i++) {
      VkMemoryPropertyFlags mask = 0;
      if(type & HOST_VISIBLE) {
        mask = mask | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      }
      if(type & HOST_COHERENT) {
        mask = mask | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      }
      if(type & DEVICE_LOCAL) {
        mask = mask | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      }

      if(mask & memory_properties.memoryTypes[i].propertyFlags) {
        index = i;
        break;
      }
    }
    
    return index;
  }

  Buffer* create_buffer(Buffer_Create_Info info, VmaPool pool, VmaAllocator allocator, Engine* engine) {
    VkResult res;
    Buffer* buffer = new Buffer;
    buffer->id = info.id;
    buffer->size = info.size;
    buffer->raw = info.raw;
    buffer->type = info.type;

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.pool = pool;

    VkBufferUsageFlags flag = 0;
    if(info.type & VERTEX) {
      flag = flag | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(info.type & INDEX) {
      flag = flag | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(info.type & UNIFORM) {
      flag = flag | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if(info.type & TRANSFER) {
      flag = flag | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    VkSharingMode mode;
    if(info.share) {
      mode = VK_SHARING_MODE_CONCURRENT;
    }
    else {
      mode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = 0;
    buffer_create_info.size = info.size;
    buffer_create_info.usage = flag;
    buffer_create_info.sharingMode = mode;

    res = vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer->buffer, &buffer->allocation, nullptr);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The buffer could not be created. Shutting Down." << std::endl;
      throw;
    }

    if(info.raw) {
      vmaMapMemory(allocator, buffer->allocation, &buffer->memory);
    }

    return buffer;
  }
  void destroy_buffer(Buffer* buffer, VmaAllocator allocator, Engine* engine) {
    if(buffer->raw) {
      vmaUnmapMemory(allocator, buffer->allocation);
    }

    vmaDestroyBuffer(allocator, buffer->buffer, buffer->allocation);
    delete buffer;
  }

  void copy_floats_to_buffer(std::vector<float> data, Buffer* buffer) {
    bool size_check = buffer->size >= data.size()*sizeof(float);
    bool raw_check = buffer->raw;

    if(!raw_check) {
      std::cerr << "The buffer is not a raw buffer so it is not mapped. Shutting down." << std::endl;
      throw;
    }

    if(!size_check) {
      std::cerr << "The buffer is not big enough to fit the data you want to push to it. Shutting down." << std::endl;
      throw;
    }

    memcpy(buffer->memory, (void*)data.data(), data.size()*sizeof(float));
  }
  void copy_ints_to_buffer(std::vector<uint32_t> data, Buffer* buffer) {
    bool size_check = buffer->size >= data.size()*sizeof(uint32_t);
    bool raw_check = buffer->raw;

    if(!raw_check) {
      std::cerr << "The buffer is not a raw buffer so it is not mapped. Shutting down." << std::endl;
      throw;
    }

    if(!size_check) {
      std::cerr << "The buffer is not big enough to fit the data you want to push to it. Shutting down." << std::endl;
      throw;
    }

    memcpy(buffer->memory, (void*)data.data(), data.size()*sizeof(uint32_t));
  }
}