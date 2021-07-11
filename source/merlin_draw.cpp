#include "../include/merlin_draw.hpp"
#include "../include/merlin_help.hpp"

#include <map>
#include <thread>
#include <iostream>

namespace merlin {
  Graph create_graph(Graph_Init init, Window* window) {
    Graph graph = {};
    VkResult res;
    VkDevice device = window->linked_engine->device;

    graph.linked = true;
    graph.id = init.id;
    graph.linked_window = window;

    // create the image views for rendering
    graph.views.resize(window->image_count);
    for(uint32_t i=0; i<window->image_count; i++) {
      VkImageViewCreateInfo view_create_info = {};
      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.pNext = nullptr;
      view_create_info.flags = 0;
      view_create_info.image = window->images[i];
      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_create_info.format = window->format.format;
      view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      view_create_info.subresourceRange.baseMipLevel = 0;
      view_create_info.subresourceRange.levelCount = 1;
      view_create_info.subresourceRange.baseArrayLayer = 0;
      view_create_info.subresourceRange.layerCount = 1;

      res = vkCreateImageView(device, &view_create_info, nullptr, &graph.views[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The image views could not be created. Shutting Down." << std::endl;
        throw;
      }
    }

    VkCommandPoolCreateInfo draw_pool_crate_info = {};
    draw_pool_crate_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    draw_pool_crate_info.pNext = nullptr;
    draw_pool_crate_info.flags = 0;
    draw_pool_crate_info.queueFamilyIndex = window->linked_engine->graphics_index;

    res = vkCreateCommandPool(device, &draw_pool_crate_info, nullptr, &graph.draw_pool);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The draw pool could not be created. Shutting Down." << std::endl;
      throw;
    }

    VkCommandPoolCreateInfo transfer_pool_create_info = {};
    transfer_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_pool_create_info.pNext = nullptr;
    transfer_pool_create_info.flags = 0;
    transfer_pool_create_info.queueFamilyIndex = window->linked_engine->transfer_index;

    res = vkCreateCommandPool(device, &transfer_pool_create_info, nullptr, &graph.transfer_pool);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The transfer pool could not be created. Shutting Down." << std::endl;
      throw;
    }

    //set up the unloaded data buffer
    if(init.activated) {
      graph.unloaded_states.push_back(init.active);
    }
    for(auto state_init : init.loaded) {
      graph.unloaded_states.push_back(state_init);
    }
    for(auto state_init : init.unloaded) {
      graph.unloaded_states.push_back(state_init);
    }

    // now load the loaded data
    if(init.activated) {
      int active_id = init.active.id;
      for(uint32_t i=0; i<graph.unloaded_states.size(); i++) {
        if(active_id == graph.unloaded_states[i].id) {
          load_state(active_id, &graph);
          activate_state(active_id, &graph);
          break;
        }
      }
    }

    for(uint32_t i=0; i<init.loaded.size(); i++) {
      bool found = false;
      for(uint32_t j=0; j<graph.unloaded_states.size(); j++) {
        if(graph.unloaded_states[j].id == init.loaded[i].id) {
          load_state(init.loaded[i].id, &graph);
          found = true;
          break;
        }
      }
      if(!found) {
        std::cerr << "The state with id: " << init.loaded[i].id << "could not be found." << std::endl;
      }
    }
    return graph;
  }
  void destroy_graph(Graph graph) {
    VkDevice device = graph.linked_window->linked_engine->device;
    vkDeviceWaitIdle(device);

    for(auto view : graph.views) {
      vkDestroyImageView(device, view, nullptr);
    }
    for(auto state : graph.loaded_states) {
      unload_state(state.id, &graph);
    }

    vkDestroyCommandPool(device, graph.draw_pool, nullptr);
    vkDestroyCommandPool(device, graph.transfer_pool, nullptr);
  }

  void load_state(int id, Graph* graph) {
    VkResult res;

    State_Init state_init;
    bool state_found = false;
    for(uint32_t i=0; i<graph->unloaded_states.size(); i++) {
      int search_id = graph->unloaded_states[i].id;
      
      if(id == search_id) {
        state_init = graph->unloaded_states[i];
        state_found = true;
        break;
      }
    }
    if(!state_found) {
      std::cerr << "The unloaded state could not be found. Shutting Down." << std::endl;
      throw;
    }

    // load relevent data here
    VkDevice device = graph->linked_window->linked_engine->device;

    // the construction of the state begins here
    State state = {};
    state.id = state_init.id;

    state.vertex_module = help::create_shader_module(state_init.shader.vertex_path, device);
    state.fragment_module = help::create_shader_module(state_init.shader.fragment_path, device);
    state.geometry = state_init.shader.geometry;
    if(state.geometry) {
      state.geometry_module = help::create_shader_module(state_init.shader.geometry_path, device);
    }

    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.pNext = nullptr;
    vertex_stage_info.flags = 0;
    vertex_stage_info.pName = state_init.shader.name.c_str();
    vertex_stage_info.module = state.vertex_module;
    vertex_stage_info.pSpecializationInfo = nullptr;
    vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo fragment_stage_info = {};
    fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage_info.pNext = nullptr;
    fragment_stage_info.flags = 0;
    fragment_stage_info.pName = state_init.shader.name.c_str();
    fragment_stage_info.module = state.fragment_module;
    fragment_stage_info.pSpecializationInfo = nullptr;
    fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineShaderStageCreateInfo geometry_stage_info = {};
    if(state.geometry) {
      geometry_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      geometry_stage_info.pNext = nullptr;
      geometry_stage_info.flags = 0;
      geometry_stage_info.pName = state_init.shader.name.c_str();
      geometry_stage_info.module = state.geometry_module;
      geometry_stage_info.pSpecializationInfo = nullptr;
      geometry_stage_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    }

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos = {vertex_stage_info, fragment_stage_info};
    if(state.geometry) {
      shader_stage_infos.push_back(geometry_stage_info);
    }

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    if(state_init.input.input_data) {
      bindings.resize(state_init.input.bindings.size());
      for(uint32_t i=0; i<bindings.size(); i++) {
        bindings[i].binding = state_init.input.bindings[i].binding;
        bindings[i].stride = state_init.input.bindings[i].stride;
        switch(state_init.input.bindings[i].rate)
        {
          case RATE_VERTEX:
            bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            break;
          case RATE_INTANCE:
            bindings[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            break;
        }
      }

      attributes.resize(state_init.input.attributes.size());
      for(uint32_t i=0; i<attributes.size(); i++) {
        attributes[i].binding = state_init.input.attributes[i].binding;
        attributes[i].location = state_init.input.attributes[i].location;
        switch(state_init.input.attributes[i].loc_format)
        {
          case FVEC1:
            attributes[i].format = VK_FORMAT_R32_SFLOAT;
            break;
          case FVEC2:
            attributes[i].format = VK_FORMAT_R32G32_SFLOAT;
            break;
          case FVEC3:
            attributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
            break;
          case FVEC4:
            attributes[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
          case DVEC1:
            attributes[i].format = VK_FORMAT_R64_SFLOAT;
            break;
          case DVEC2:
            attributes[i].format = VK_FORMAT_R64G64_SFLOAT;
            break;
          case DVEC3:
            attributes[i].format = VK_FORMAT_R64G64B64_SFLOAT;
            break;
          case DVEC4:
            attributes[i].format = VK_FORMAT_R64G64B64A64_SFLOAT;
            break;
        }
        attributes[i].offset = state_init.input.attributes[i].offset;
      }

      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.pNext = nullptr;
      vertex_input_info.flags = 0;
      vertex_input_info.vertexBindingDescriptionCount = bindings.size();
      vertex_input_info.pVertexBindingDescriptions = bindings.data();
      vertex_input_info.vertexAttributeDescriptionCount = attributes.size();
      vertex_input_info.pVertexAttributeDescriptions = attributes.data();
    }
    else {
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.pNext = nullptr;
      vertex_input_info.flags = 0;
      vertex_input_info.vertexBindingDescriptionCount = 0;
      vertex_input_info.pVertexBindingDescriptions = nullptr;
      vertex_input_info.vertexAttributeDescriptionCount = 0;
      vertex_input_info.pVertexAttributeDescriptions = nullptr;
    }

    VkPipelineInputAssemblyStateCreateInfo input_assembly = help::create_input_assemply(state_init.fixed_functions.topology, state_init.fixed_functions.reuse);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) graph->linked_window->extent_2d.width;
    viewport.height = (float) graph->linked_window->extent_2d.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = graph->linked_window->extent_2d;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;
    viewport_state.flags = 0;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    switch(state_init.fixed_functions.mode)
    {
      case FILL:
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        break;
      case LINE:
        rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
        break;
      case POINT:
        rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
        break;
    }
    rasterizer.lineWidth = state_init.fixed_functions.line_width;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisample = {};
    if(state_init.multisampling.enable) {
    }
    else {
      multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisample.pNext = nullptr;
      multisample.flags = 0;
      multisample.sampleShadingEnable = VK_FALSE;
      multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisample.minSampleShading = 1.0f;
      multisample.pSampleMask = nullptr;
      multisample.alphaToCoverageEnable = VK_FALSE;
      multisample.alphaToOneEnable = VK_FALSE;
    }

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.pNext = nullptr;
    color_blending.flags = 0;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    if(state_init.uniform.uniform) {

    }
    else {
      pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipeline_layout_info.pNext = nullptr;
      pipeline_layout_info.flags = 0;
      pipeline_layout_info.setLayoutCount = 0;
      pipeline_layout_info.pSetLayouts = nullptr;
      pipeline_layout_info.pushConstantRangeCount = 0;
      pipeline_layout_info.pPushConstantRanges = nullptr;
    }
    res = vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &state.pipeline_layout);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The Pipeline layout could not be created. Shutting Down." << std::endl;
      throw;
    }

    std::vector<VkAttachmentDescription> descriptions(state_init.render_pass.attachments.size());
    for(uint32_t i=0; i<descriptions.size(); i++) {
      Attachment attachment_init = state_init.render_pass.attachments[i];
      descriptions[i] = help::create_attachment_description(
        attachment_init.sample_count,
        graph->linked_window->format.format,
        attachment_init.data_ops,
        attachment_init.stencil_ops,
        attachment_init._layouts
      );
    }

    std::vector<VkAttachmentReference> depth_stencil_refrences(state_init.render_pass.subpasses.size());
    std::vector<std::vector<VkAttachmentReference>> color_refrences(state_init.render_pass.subpasses.size());
    std::vector<std::vector<VkAttachmentReference>> input_refrences(state_init.render_pass.subpasses.size());
    std::vector<std::vector<uint32_t>> preserve_refrences(state_init.render_pass.subpasses.size());
    std::vector<std::vector<VkAttachmentReference>> resolve_refrences(state_init.render_pass.subpasses.size());
    std::vector<VkSubpassDescription> subpasses(state_init.render_pass.subpasses.size());
    for(uint32_t i=0; i<subpasses.size(); i++) {
      Subpass subpass_init = state_init.render_pass.subpasses[i];

      if(subpass_init.depth_stencil) {
        depth_stencil_refrences[i].attachment = (uint32_t)subpass_init.depth_stencil_attachment;
        depth_stencil_refrences[i].layout = help::choose_layout(state_init.render_pass.attachments[subpass_init.depth_stencil_attachment]._layouts);
      }
      color_refrences[i].resize(subpass_init.color_attachments.size());
      for(uint32_t j=0; j<color_refrences[i].size(); j++) {
        color_refrences[i][j].attachment = (uint32_t)subpass_init.color_attachments[j];
        color_refrences[i][j].layout = help::choose_layout(state_init.render_pass.attachments[subpass_init.color_attachments[j]]._layouts);
      }
      input_refrences[i].resize(subpass_init.input_attachments.size());
      for(uint32_t j=0; j<input_refrences[i].size(); j++) {
        input_refrences[i][j].attachment = (uint32_t)subpass_init.input_attachments[j];
        input_refrences[i][j].layout = help::choose_layout(state_init.render_pass.attachments[subpass_init.input_attachments[j]]._layouts);
      }
      preserve_refrences[i].resize(subpass_init.preserve_attachments.size());
      for(uint32_t j=0; j<preserve_refrences[i].size(); j++) {
        preserve_refrences[i][j] = (uint32_t)subpass_init.preserve_attachments[j];
      }
      resolve_refrences[i].resize(subpass_init.resolve_attachments.size());
      for(uint32_t j=0; j<resolve_refrences[i].size(); j++) {
        resolve_refrences[i][j].attachment = (uint32_t)subpass_init.resolve_attachments[j];
        resolve_refrences[i][j].layout = help::choose_layout(state_init.render_pass.attachments[subpass_init.resolve_attachments[j]]._layouts);
      }

      subpasses[i].flags = 0;
      switch(subpass_init.point)
      {
        case GRAPHICS:
          subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
          break;
      }
      subpasses[i].colorAttachmentCount = color_refrences[i].size();
      subpasses[i].pColorAttachments = color_refrences[i].data();
      subpasses[i].inputAttachmentCount = input_refrences[i].size();
      subpasses[i].pInputAttachments = input_refrences[i].data();
      subpasses[i].preserveAttachmentCount = preserve_refrences[i].size();
      subpasses[i].pPreserveAttachments = preserve_refrences[i].data();
      subpasses[i].pResolveAttachments = resolve_refrences[i].data();
      if(subpass_init.depth_stencil) {
        subpasses[i].pDepthStencilAttachment = &depth_stencil_refrences[i];
      }
    }

    std::vector<VkSubpassDependency> dependencies = {}; // implement dependencies later
    for(uint32_t i=0; i<state_init.render_pass.dependencies.size(); i++) {
      Dependency dependency = state_init.render_pass.dependencies[i];

      VkSubpassDependency subpass_dependency = {};

      dependencies.push_back(subpass_dependency);
    }
    VkSubpassDependency render_dependency = {}; // fix for more than one subpass
    render_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    render_dependency.dstSubpass = 0;
    render_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_dependency.srcAccessMask = 0;
    render_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies.push_back(render_dependency);

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.flags = 0;
    render_pass_info.attachmentCount = descriptions.size();
    render_pass_info.pAttachments = descriptions.data();
    render_pass_info.subpassCount = subpasses.size();
    render_pass_info.pSubpasses = subpasses.data();
    render_pass_info.dependencyCount = dependencies.size();
    render_pass_info.pDependencies = dependencies.data();

    res = vkCreateRenderPass(device, &render_pass_info, nullptr, &state.render_pass);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The render pass faild to create. Shutting Down." << std::endl;
      throw;
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = nullptr;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stageCount = shader_stage_infos.size();
    pipeline_create_info.pStages = shader_stage_infos.data();
    pipeline_create_info.pVertexInputState = &vertex_input_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterizer;
    pipeline_create_info.pMultisampleState = &multisample;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blending;
    pipeline_create_info.pDynamicState = nullptr;
    pipeline_create_info.layout = state.pipeline_layout;
    pipeline_create_info.renderPass = state.render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &state.pipeline);
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The pipeline could not be created. Shutting Down." << std::endl;
      throw;
    }

    state.vertex_buffers.resize(state_init.input.bindings.size());
    state.index_buffers.resize(state_init.input.bindings.size());
    for(uint32_t i=0; i<state.vertex_buffers.size(); i++) {
      size_t instance_count = state_init.input.instance_count;
      size_t vertex_count = state_init.input.vertex_count;
      size_t binding_size = state_init.input.bindings[i].stride;

      VkBufferCreateInfo vertex_buffer_create_info = {};
      vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      vertex_buffer_create_info.pNext = nullptr;
      vertex_buffer_create_info.flags = 0;
      vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      vertex_buffer_create_info.size = instance_count*vertex_count*binding_size;

      res = vkCreateBuffer(graph->linked_window->linked_engine->device, &vertex_buffer_create_info, nullptr, &state.vertex_buffers[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "A vertex buffer could not be created. Shutting Down." << std::endl;
        throw;
      }
    }

    state.framebuffers.resize(graph->views.size());
    for(uint32_t i=0; i<state.framebuffers.size(); i++) {
      VkFramebufferCreateInfo framebuffer_create_info = {};
      framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebuffer_create_info.pNext = nullptr;
      framebuffer_create_info.flags = 0;
      framebuffer_create_info.renderPass = state.render_pass;
      framebuffer_create_info.attachmentCount = 1;
      framebuffer_create_info.pAttachments = &graph->views[i];
      framebuffer_create_info.width = graph->linked_window->extent_2d.width;
      framebuffer_create_info.height = graph->linked_window->extent_2d.height;
      framebuffer_create_info.layers = 1;

      res = vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &state.framebuffers[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The framebuffer could not be created. Shutting Down." << std::endl;
        throw;
      }
    }

    state.draw_buffers.resize(state.framebuffers.size());

    VkCommandBufferAllocateInfo allocation_info = {};
    allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocation_info.pNext = nullptr;
    allocation_info.commandPool = graph->draw_pool;
    allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocation_info.commandBufferCount = (uint32_t) state.draw_buffers.size();

    res = vkAllocateCommandBuffers(device, &allocation_info, state.draw_buffers.data());
    if(res != VK_SUCCESS) {
      std::cout << res << std::endl;
      std::cerr << "The draw buffers could not be allocated. Shutting Down." << std::endl;
      throw;
    }

    for(uint32_t i=0; i<state.draw_buffers.size(); i++) {
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.pNext = nullptr;
      begin_info.flags = 0;

      res = vkBeginCommandBuffer(state.draw_buffers[i], &begin_info);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The draw buffer could not begin recording. Shutting Down." << std::endl;
        throw;
      }

      VkRenderPassBeginInfo render_pass_begin_info = {};
      render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      render_pass_begin_info.pNext = 0;
      render_pass_begin_info.renderPass = state.render_pass;
      render_pass_begin_info.framebuffer = state.framebuffers[i];
      render_pass_begin_info.renderArea.offset = {0, 0};
      render_pass_begin_info.renderArea.extent = graph->linked_window->extent_2d;

      VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
      render_pass_begin_info.clearValueCount = 1;
      render_pass_begin_info.pClearValues = &clear_color;

      vkCmdBeginRenderPass(state.draw_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(state.draw_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);

        vkCmdDraw(state.draw_buffers[i], state_init.input.vertex_count, state_init.input.instance_count, 0, 0);

      vkCmdEndRenderPass(state.draw_buffers[i]);

      res = vkEndCommandBuffer(state.draw_buffers[i]);
      if(res != VK_SUCCESS) {
        std::cout << res << std::endl;
        std::cerr << "The draw buffer did not finish recording. Shutting Down." << std::endl;
        throw;
      }
    }

    uint32_t size = graph->loaded_states.size();
    graph->loaded_states.resize(size + 1);

    uint32_t last_place = graph->loaded_states.size() - 1;
    graph->loaded_states[last_place] = state;
  }
  void unload_state(int id, Graph* graph) {
    State state;
    std::vector<State>::iterator state_it;
    bool state_found = false;
    for(std::vector<State>::iterator it=graph->loaded_states.begin(); it<graph->loaded_states.end(); it++) {
      if((*it).id == id) {
        state = *it;
        state_it = it;
        state_found = true;
        break;
      }
    }
    if(!state_found) {
      std::cerr << "The state could not be found. Shutting Down." << std::endl;
      throw;
    }

    VkDevice device = graph->linked_window->linked_engine->device;

    vkDestroyShaderModule(device, state.vertex_module, nullptr);
    vkDestroyShaderModule(device, state.fragment_module, nullptr);
    if(state.geometry) {
      vkDestroyShaderModule(device, state.geometry_module, nullptr);
    }

    for(uint32_t i=0; i<state.vertex_buffers.size(); i++) {
      vkDestroyBuffer(device, state.vertex_buffers[i], nullptr);
    }

    for(auto framebuffer : state.framebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyPipelineLayout(device, state.pipeline_layout, nullptr);
    vkDestroyRenderPass(device, state.render_pass, nullptr);
    vkDestroyPipeline(device, state.pipeline, nullptr);
    
    graph->loaded_states.erase(state_it);
  }
  void activate_state(int id, Graph* graph) {
    bool found = false;
    for(uint32_t i=0; i<graph->loaded_states.size(); i++) {
      int search_id = graph->loaded_states[i].id;
      if(search_id == id) {
        graph->activated = true;
        graph->active_state = &graph->loaded_states[i];

        found = true;
        break;
      }
    }
    if(!found) {
      std::cerr << "The state could not be found to be activated. Did you load it?" << std::endl;
      throw;
    }
  }
  void deactivate_state(Graph* graph) {
    graph->activated = false;
    graph->active_state = nullptr;
  }

  void subdraw(std::vector<Graph*> graphs, Window* window, Engine* engine) {
    VkResult res;
    vkWaitForFences(engine->device, 1, &window->in_flight_fences[window->current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    vkAcquireNextImageKHR(engine->device, window->swapchain, UINT64_MAX, window->image_available_semaphores[window->current_frame], VK_NULL_HANDLE, &image_index);

    if(window->images_in_flight[image_index] != VK_NULL_HANDLE) {
      vkWaitForFences(engine->device, 1, &window->images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    window->images_in_flight[image_index] = window->in_flight_fences[window->current_frame];

    std::vector<VkSubmitInfo> submit_infos(graphs.size());
    for(uint32_t i=0; i<graphs.size(); i++) {
      submit_infos[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_infos[i].pNext = nullptr;

      VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      submit_infos[i].waitSemaphoreCount = 1;
      submit_infos[i].pWaitSemaphores = &window->image_available_semaphores[window->current_frame];
      submit_infos[i].pWaitDstStageMask = waitStages;
      submit_infos[i].commandBufferCount = 1;
      submit_infos[i].pCommandBuffers = &graphs[i]->active_state->draw_buffers[image_index];
      submit_infos[i].signalSemaphoreCount = 1;
      submit_infos[i].pSignalSemaphores = &window->render_finished_semaphores[window->current_frame];
    }

    vkResetFences(engine->device, 1, &window->in_flight_fences[window->current_frame]);

    res = vkQueueSubmit(engine->graphics_queue, submit_infos.size(), submit_infos.data(), window->in_flight_fences[window->current_frame]);
    if(res != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &window->render_finished_semaphores[window->current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &window->swapchain;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(engine->present_queue, &present_info);

    window->current_frame = (window->current_frame + 1) % window->max_frames;
  }

  void draw(std::vector<Graph*> graphs) {
    // sort out how many windows have objects in the graphs list
    std::map<std::string, int> graph_counts;
    std::map<std::string, std::vector<Graph*>> graph_pointers;
    for(auto graph_ptr : graphs) {
      std::string title = graph_ptr->linked_window->title;
      if(graph_counts.find(title) != graph_counts.end()) {
        graph_counts[title] += 1;
        graph_pointers[title].push_back(graph_ptr);
      }
      else {
        graph_counts[title] = 1;
        graph_pointers[title] = { graph_ptr };
      }
    }

    // at the moment we draw all of this synchronasly but later this will be changed for preformence
    for(auto key_value_pair : graph_pointers) {
      auto graph_pointers = key_value_pair.second;
      auto window = graph_pointers[0]->linked_window;
      auto engine = window->linked_engine;

      subdraw(graph_pointers, window, engine);
    }
  }
}