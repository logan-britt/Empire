#ifndef MERLIN_MEMORY
#define MERLIN_MEMORY

#include <array>
#include <vector>
#include <iostream>

#include <vulkan/vulkan.h>
#include "../include/vk_mem_alloc.h"

#include "../include/merlin.hpp"

namespace merlin
{
  typedef size_t memory_size;
  typedef uint8_t buffer_type;
  typedef uint8_t memory_type;

  extern memory_size BYTE, KILOBYTE, MEGABYTE, GIGABYTE;
  extern buffer_type VERTEX, INDEX, UNIFORM, TRANSFER;
  extern memory_type HOST_VISIBLE, HOST_COHERENT, DEVICE_LOCAL;

  enum share_mode{EXCLUSIVE, CONCURRENT};

  struct Buffer_Create_Info
  {
    int id;
    size_t size;
    bool raw;
    bool share;
    buffer_type type;
  };
  struct Buffer
  {
    int id;
    buffer_type type;
    size_t size;

    bool raw;
    void* memory;

    VkBuffer buffer;
    VmaAllocation allocation;
  };

  uint32_t find_memory_index(memory_type type, VkPhysicalDevice physical_device);
  
  Buffer* create_buffer(Buffer_Create_Info info, VmaPool pool, VmaAllocator allocator, Engine* engine);
  void destroy_buffer(Buffer* buffer, VmaAllocator allocator, Engine* engine);

  void copy_floats_to_buffer(std::vector<float> data, Buffer* buffer);
  void copy_ints_to_buffer(std::vector<uint32_t> data, Buffer* buffer);
}

#endif