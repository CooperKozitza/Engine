#include "../include/application.hpp"

eng::application *eng::application::app;

std::mutex eng::application::creation_mutex;

eng::application *eng::application::create(const vec2<unsigned int> &res,
                                           const char *title) {
  std::lock_guard<std::mutex> guard(creation_mutex);
  if (!app) {
    app = new application(res, title);
  }

  return app;
}

eng::application::application(const vec2<unsigned int> res, const char *name)
    : m_window_details(res, name), m_window(nullptr), name(name),
      m_instance(new instance()), m_surface(new surface()),
      m_device(new device()), m_swap_chain(new swap_chain()) {}

void eng::application::create_graphics_pipeline() {
  std::vector<shader *>::iterator vert_shader_pos =
      std::find_if(shaders.begin(), shaders.end(),
                   [](shader *x) { return x->get_type() == VERTEX; });

  std::vector<shader *>::iterator frag_shader_pos =
      std::find_if(shaders.begin(), shaders.end(),
                   [](shader *x) { return x->get_type() == FRAGMENT; });

  if (vert_shader_pos == shaders.end() || frag_shader_pos == shaders.end()) {
    throw std::runtime_error("could not find required shaders");
  }

  VkShaderModule vert_shder_module =
      (*vert_shader_pos)->create_shader_module(m_device);
  VkShaderModule frag_shder_module =
      (*frag_shader_pos)->create_shader_module(m_device);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shder_module;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shder_module;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                     frag_shader_stage_info};

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)m_swap_chain->get_extent().width;
  viewport.height = (float)m_swap_chain->get_extent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = m_swap_chain->get_extent();

  std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount =
      static_cast<unsigned int>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = nullptr;            // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f; // Optional
  color_blending.blendConstants[1] = 0.0f; // Optional
  color_blending.blendConstants[2] = 0.0f; // Optional
  color_blending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;            // Optional
  pipeline_layout_info.pSetLayouts = nullptr;         // Optional
  pipeline_layout_info.pushConstantRangeCount = 0;    // Optional
  pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

  if (vkCreatePipelineLayout(m_device->get(), &pipeline_layout_info, nullptr,
                             &pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = nullptr; // Optional
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state;

  pipeline_info.layout = pipeline_layout;

  pipeline_info.renderPass = render_pass;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1;              // Optional

  if (vkCreateGraphicsPipelines(m_device->get(), VK_NULL_HANDLE, 1,
                                &pipeline_info, nullptr,
                                &graphics_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  (*vert_shader_pos)->destroy_shader_module(m_device);
  (*frag_shader_pos)->destroy_shader_module(m_device);

  std::cout << "Created Graphics Pipeline" << std::endl;
}

void eng::application::create_render_pass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = m_swap_chain->get_image_format();
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  if (vkCreateRenderPass(m_device->get(), &render_pass_info, nullptr,
                         &render_pass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void eng::application::uninitialize_vulkan() {
  vkDestroyPipeline(m_device->get(), graphics_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device->get(), pipeline_layout, nullptr);
  vkDestroyRenderPass(m_device->get(), render_pass, nullptr);
}

eng::application::~application() {
  if (is_running()) {
    stop();
  }

  uninitialize_vulkan();

  delete m_swap_chain;
  delete m_device;
  delete m_surface;
  delete m_instance;

  for (auto s : shaders) {
    if (s) {
      delete s;
    }
  }

  delete m_window;
}

void eng::application::set_shader(const char *file_path, shader_type type) {
  if (!shaders.empty()) {
    std::vector<shader *>::iterator duplacate =
        std::find_if(shaders.begin(), shaders.end(),
                     [&type](shader *x) { return x->get_type() == type; });

    if (duplacate != shaders.end()) {
      std::cout << "Replacing " << typeid(type).name()
                << " Shader With Shader At: " << file_path << std::endl;

      delete *duplacate;
      *duplacate = new shader(file_path, type);

      return;
    }
  }

  shaders.push_back(new shader(file_path, type));
}

void eng::application::start() {
  bool has_vert_shaders =
      std::any_of(shaders.begin(), shaders.end(),
                  [](shader *x) { return x->get_type() == VERTEX; });
  bool has_frag_shaders =
      std::any_of(shaders.begin(), shaders.end(),
                  [](shader *x) { return x->get_type() == FRAGMENT; });

  if (!has_vert_shaders || !has_frag_shaders) {
    std::cerr << "Vertex and Fragment Shaders Not Set! Set Shaders With "
                 "application::set_shader(...) Before Calling start()"
              << std::endl;
    return;
  }

  running = true;
  main = std::thread([this] {
    m_window = new window(m_window_details);

    m_instance->create_instance(name);

    m_surface->create_surface(m_instance, m_window);

    m_device->create_device(m_instance, m_surface);

    m_swap_chain->create_swap_chain(m_device, m_surface);
    m_swap_chain->create_image_views();

    //create_render_pass();
    //create_graphics_pipeline();

    while (!m_window->should_close() && is_running()) {
      m_window->poll_events();

      // main loop logic
    }

    running = false;
  });
}

void eng::application::stop() {
  std::cout << "Application Stopped" << std::endl;

  running = false;
  if (main.joinable()) {
    main.join();
  }
}