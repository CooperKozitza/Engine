#include "../include/graphics_pipeline.hpp"

eng::pipeline::pipeline()
    : m_device(VK_NULL_HANDLE), m_render_pass(VK_NULL_HANDLE),
      m_descriptor_set_layout(VK_NULL_HANDLE),
      m_pipeline_layout(VK_NULL_HANDLE), m_pipeline(VK_NULL_HANDLE),
      m_shaders() {}

eng::pipeline::~pipeline() {
  if (m_descriptor_set_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
  }
  if (m_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
  }
  if (m_pipeline_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
  }
  if (m_render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);
  }

  for (std::unique_ptr<shader> &shader : m_shaders) {
    shader.reset();
  }
}

void eng::pipeline::add_shader(const char *file_path, shader_type type) {
  if (!m_shaders.empty()) {
    std::vector<std::unique_ptr<shader>>::iterator duplacate = std::find_if(
        m_shaders.begin(), m_shaders.end(),
        [&type](std::unique_ptr<shader> &x) { return x->get_type() == type; });

    if (duplacate != m_shaders.end()) {
      std::cout << "Replacing " << typeid(type).name()
                << " Shader With Shader At: " << file_path << std::endl;

      (*duplacate) = std::make_unique<shader>(file_path, type);

      return;
    }
  }

  m_shaders.push_back(std::make_unique<shader>(file_path, type));
}

void eng::pipeline::create_descriptor_set_layout(device &dev) {
  m_device = dev.get_device();

  VkDescriptorSetLayoutBinding ubo_layout_binding =
      uniform_buffer_object::get_descriptor_set_layout_binding();

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &ubo_layout_binding;

  if (vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr,
                                  &m_descriptor_set_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void eng::pipeline::create_graphics_pipeline(device &dev, swap_chain &sc) {
  std::vector<std::unique_ptr<shader>>::iterator vert_shader_pos = std::find_if(
      m_shaders.begin(), m_shaders.end(),
      [](std::unique_ptr<shader> &x) { return x->get_type() == VERTEX; });

  std::vector<std::unique_ptr<shader>>::iterator frag_shader_pos = std::find_if(
      m_shaders.begin(), m_shaders.end(),
      [](std::unique_ptr<shader> &x) { return x->get_type() == FRAGMENT; });

  if (vert_shader_pos == m_shaders.end() ||
      frag_shader_pos == m_shaders.end()) {
    throw std::runtime_error("could not find required shaders");
  }

  m_device = dev.get_device();

  VkShaderModule vert_shder_module =
      (*vert_shader_pos)->create_shader_module(dev);
  VkShaderModule frag_shder_module =
      (*frag_shader_pos)->create_shader_module(dev);

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

  auto binding_description = vertex::get_binding_properties();
  auto attribute_description = vertex::get_attribute_description();

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_description;

  vertex_input_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attribute_description.size());
  vertex_input_info.pVertexAttributeDescriptions = attribute_description.data();

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkExtent2D extent = sc.get_extent();

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)extent.width;
  viewport.height = (float)extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
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
  rasterizer.cullMode = VK_CULL_MODE_NONE;
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
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &m_descriptor_set_layout;
  pipeline_layout_info.pushConstantRangeCount = 0;    // Optional
  pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

  if (vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr,
                             &m_pipeline_layout) != VK_SUCCESS) {
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

  pipeline_info.layout = m_pipeline_layout;

  pipeline_info.renderPass = m_render_pass;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1;              // Optional

  if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &m_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  std::cout << "Created Graphics Pipeline" << std::endl;
}

void eng::pipeline::create_render_pass(device &dev, swap_chain &sc) {
  m_device = dev.get_device();

  VkAttachmentDescription color_attachment{};
  color_attachment.format = sc.get_image_format();
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

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  if (vkCreateRenderPass(m_device, &render_pass_info, nullptr,
                         &m_render_pass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

bool eng::pipeline::required_shaders_set() {
  bool has_vert_shaders =
      std::any_of(m_shaders.begin(), m_shaders.end(),
      [](std::unique_ptr<shader> &x) { return x->get_type() == VERTEX; });
  bool has_frag_shaders =
      std::any_of(m_shaders.begin(), m_shaders.end(),
      [](std::unique_ptr<shader>  &x) { return x->get_type() == FRAGMENT; });

  return has_vert_shaders && has_frag_shaders;
}