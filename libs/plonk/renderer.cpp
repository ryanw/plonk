#include "include/plonk/frame.h"
#include "include/plonk/renderer.h"
#include "include/plonk/camera.h"
#include "include/plonk/math.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

struct SimplePushConstants {
	float screen_size[2];
	float _pad0[2];
	Point3 position;
	float _pad1[1];
	Point3 direction;
	float time;
};

Renderer::Renderer(ContextPtr ctx) : ctx(ctx) {
	std::cout << "Creating Renderer\n";
	started_at = std::chrono::high_resolution_clock::now();
	vert_shader = ctx->load_shader("shaders/simple.vert.spv");
	frag_shader = ctx->load_shader("shaders/simple.frag.spv");
	create_pipeline();
}

void Renderer::draw(Camera &camera) {
	handle_resize();

	auto frame = ctx->aquire_frame();
	record_commands(camera);
	frame.present();
}

void Renderer::handle_resize() {
	if (ctx->needs_resize()) {
		ctx->update_swapchain();
	}
}

void Renderer::create_pipeline() {
	std::cout << "Creating Pipeline\n";

	VkPushConstantRange push_constant_range{
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(SimplePushConstants),
	};

	VkPipelineLayoutCreateInfo pipeline_layout_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range,
	};

	if (VK_SUCCESS != vkCreatePipelineLayout(ctx->device, &pipeline_layout_info, nullptr, &pipeline_layout)) {
		throw std::runtime_error("Failed to create Pipeline Layout");
	}

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = ctx->width(),
		.height = ctx->height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = ctx->size(),
	};

	VkPipelineViewportStateCreateInfo viewport_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	VkPipelineShaderStageCreateInfo vert_create_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vert_shader,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo frag_create_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = frag_shader,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo stages[] = {vert_create_info, frag_create_info};

	VkPipelineInputAssemblyStateCreateInfo input_assembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data(),
	};

	VkPipelineRasterizationStateCreateInfo rasterizer_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = {0.0, 0.0, 0.0, 0.0},
	};

	VkGraphicsPipelineCreateInfo pipeline_info{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = stages,
		.pVertexInputState = &vertex_input_state,
		.pInputAssemblyState = &input_assembly,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer_state,
		.pMultisampleState = &multisample_state,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &color_blend_state,
		.pDynamicState = &dynamic_state,
		.layout = pipeline_layout,
		.renderPass = nullptr,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	pipeline = ctx->create_graphics_pipeline(&pipeline_info);

	std::cout << "Created Pipeline\n";
}

void Renderer::record_commands(Camera &camera) {
	ctx->bind_pipeline(pipeline);

	auto &command_buffer = ctx->command_buffer;

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = ctx->width(),
		.height = ctx->height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = ctx->size(),
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now - started_at;
	float time = duration.count() / 1e9;
	SimplePushConstants constants{
		.screen_size = {viewport.width, viewport.height},
		.position = {camera.position.coords[0], camera.position.coords[1], camera.position.coords[2]},
		.direction = {camera.direction.coords[0], camera.direction.coords[1], camera.direction.coords[2]},
		.time = time,
	};
	vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstants), &constants);
	vkCmdDraw(command_buffer, 6, 1, 0, 0);
}

Renderer::~Renderer() {
	vkDestroyPipeline(ctx->device, pipeline, nullptr);
	vkDestroyPipelineLayout(ctx->device, pipeline_layout, nullptr);
	ctx->destroy_shader(vert_shader);
	ctx->destroy_shader(frag_shader);
}
