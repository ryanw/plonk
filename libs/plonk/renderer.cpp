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
	float screenSize[2];
	float _pad0[2];
	Point3 position;
	float _pad1[1];
	Point3 direction;
	float time;
};

Renderer::Renderer(Context &ctx) : ctx(ctx) {
	std::cout << "Creating Renderer\n";
	startedAt = std::chrono::high_resolution_clock::now();
	vertShader = ctx.loadShader("shaders/simple.vert.spv");
	fragShader = ctx.loadShader("shaders/simple.frag.spv");
	createPipeline();
}

void Renderer::draw(Camera &camera) {
	handleResize();

	auto frame = ctx.aquireFrame();
	recordCommands(camera);
	frame.present();
}

void Renderer::handleResize() {
	if (ctx.needsResize()) {
		ctx.updateSwapchain();
	}
}

void Renderer::createPipeline() {
	std::cout << "Creating Pipeline\n";

	VkPushConstantRange pushConstantRange{
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(SimplePushConstants),
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange,
	};

	if (VK_SUCCESS != vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
		throw std::runtime_error("Failed to create Pipeline Layout");
	}

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = ctx.width(),
		.height = ctx.height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = ctx.size(),
	};

	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	VkPipelineShaderStageCreateInfo vertCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShader,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo fragCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShader,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo stages[] = {vertCreateInfo, fragCreateInfo};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	VkPipelineRasterizationStateCreateInfo rasterizerState{
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

	VkPipelineMultisampleStateCreateInfo multisampleState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
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

	VkPipelineColorBlendStateCreateInfo colorBlendState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = {0.0, 0.0, 0.0, 0.0},
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = stages,
		.pVertexInputState = &vertexInputState,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizerState,
		.pMultisampleState = &multisampleState,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendState,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout,
		.renderPass = nullptr,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	pipeline = ctx.createGraphicsPipeline(&pipelineInfo);

	std::cout << "Created Pipeline\n";
}

void Renderer::recordCommands(Camera &camera) {
	ctx.bindPipeline(pipeline);

	auto &commandBuffer = ctx.commandBuffer;

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = ctx.width(),
		.height = ctx.height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = ctx.size(),
	};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now - startedAt;
	float time = duration.count() / 1e9;
	SimplePushConstants constants{
		.screenSize = {viewport.width, viewport.height},
		.position = {camera.position.coords[0], camera.position.coords[1], camera.position.coords[2]},
		.direction = {camera.direction.coords[0], camera.direction.coords[1], camera.direction.coords[2]},
		.time = time,
	};
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstants), &constants);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}

Renderer::~Renderer() {
	vkDestroyPipeline(ctx.device, pipeline, nullptr);
	vkDestroyPipelineLayout(ctx.device, pipelineLayout, nullptr);
	ctx.destroyShader(vertShader);
	ctx.destroyShader(fragShader);
}
