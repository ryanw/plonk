#include "include/plonk/renderer.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

struct SimplePushConstants {
	float time;
};

Renderer::Renderer(Context *ctx) : ctx(ctx) {
	std::cout << "Creating Renderer\n";
	startedAt = std::chrono::high_resolution_clock::now();
	vertShader = ctx->loadShader("shaders/simple.vert.spv");
	fragShader = ctx->loadShader("shaders/simple.frag.spv");
	createRenderPass();
	createPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
}

void Renderer::draw() {
	vkWaitForFences(ctx->device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(ctx->device, 1, &inFlightFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(ctx->device, ctx->getSwapchain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	vkResetCommandBuffer(commandBuffer, 0);
	recordCommands(imageIndex);

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	if (VK_SUCCESS != vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo, inFlightFence)) {
		throw std::runtime_error("Failed to submit queue");
	}

	present(imageIndex);
}

void Renderer::createRenderPass() {
	std::cout << "Creating Render Pass\n";
	VkAttachmentDescription colorAttachment{
		.format = ctx->format(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
	};

	if (VK_SUCCESS != vkCreateRenderPass(ctx->device, &renderPassCreateInfo, nullptr, &renderPass)) {
		throw std::runtime_error("Failed to create RenderPass");
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

	if (VK_SUCCESS != vkCreatePipelineLayout(ctx->device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
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
		.renderPass = renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	if (VK_SUCCESS != vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)) {
		throw std::runtime_error("Failed to create Pipeline");
	}
}

void Renderer::createFramebuffers() {
	auto count = ctx->swapchainImageCount();
	printf("Creating %d Framebuffers\n", count);

	framebuffers.resize(count);

	for (int i = 0; i < count; i++) {
		printf("Framebuffer: %d\n", i);
		auto imageView = ctx->getSwapchainImageView(i);
		if (!imageView) {
			throw std::runtime_error("Missing image view");
		}

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = &imageView,
			.width = (uint32_t)ctx->width(),
			.height = (uint32_t)ctx->height(),
			.layers = 1,
		};

		if (VK_SUCCESS != vkCreateFramebuffer(ctx->device, &framebufferInfo, nullptr, &framebuffers[i])) {
			throw std::runtime_error("Failed to create Framebuffer");
		}
	}
}

void Renderer::createCommandPool() {
	printf("Creating command pool\n");
	auto queueFamilyIndex = ctx->getGraphicsQueueFamilyIndex();
	if (!queueFamilyIndex.has_value()) {
		throw std::runtime_error("No queue family index defined");
	}
	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndex.value(),
	};

	if (VK_SUCCESS != vkCreateCommandPool(ctx->device, &poolInfo, nullptr, &commandPool)) {
		throw std::runtime_error("Failed to create Command Pool");
	}
}

void Renderer::createCommandBuffer() {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	if (VK_SUCCESS != vkAllocateCommandBuffers(ctx->device, &allocInfo, &commandBuffer)) {
		throw std::runtime_error("Failed to allocate command buffer");
	}
}

void Renderer::recordCommands(uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr,
	};
	if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
		throw std::runtime_error("Failed to start command recording");
	}

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer = framebuffers[imageIndex],
		.renderArea =
			{
				.offset = {0, 0},
				.extent = ctx->size(),
			},
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = ctx->width(),
		.height = ctx->height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = ctx->size(),
	};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);



	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now - startedAt;
	float time = duration.count() / 1e9;
	SimplePushConstants constants {
		.time = time,
	};
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstants), &constants);
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
		throw std::runtime_error("Failed to record command buffer");
	}
}

void Renderer::present(uint32_t imageIndex) {
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

	VkSwapchainKHR swapchains[] = {ctx->getSwapchain()};
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &imageIndex,
		.pResults = nullptr,
	};
	vkQueuePresentKHR(ctx->presentQueue, &presentInfo);
	vkDeviceWaitIdle(ctx->device);
}

void Renderer::createSyncObjects() {
	printf("Creating sync objects\n");

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	if (VK_SUCCESS != vkCreateSemaphore(ctx->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore)) {
		throw std::runtime_error("Failed to create image available semaphore");
	}
	if (VK_SUCCESS != vkCreateSemaphore(ctx->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore)) {
		throw std::runtime_error("Failed to create render finished semaphore");
	}
	if (VK_SUCCESS != vkCreateFence(ctx->device, &fenceInfo, nullptr, &inFlightFence)) {
		throw std::runtime_error("Failed to create in-flight fence");
	}
}

Renderer::~Renderer() {
	vkDestroySemaphore(ctx->device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(ctx->device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(ctx->device, inFlightFence, nullptr);
	vkDestroyCommandPool(ctx->device, commandPool, nullptr);
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(ctx->device, framebuffer, nullptr);
	}
	vkDestroyPipeline(ctx->device, pipeline, nullptr);
	vkDestroyPipelineLayout(ctx->device, pipelineLayout, nullptr);
	vkDestroyRenderPass(ctx->device, renderPass, nullptr);
	ctx->destroyShader(vertShader);
	ctx->destroyShader(fragShader);
}
