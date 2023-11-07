#pragma once

#include "context.h"
#include <chrono>

class Renderer {
public:
	Renderer(Context &ctx);
	~Renderer();
	void draw();

private:
	Context &ctx;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
	std::vector<VkFramebuffer> framebuffers;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
	std::chrono::time_point<std::chrono::high_resolution_clock> startedAt;

	void rebuildFramebuffers();
	void handleResize();
	void createRenderPass();
	void createPipeline();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();
	void recordCommands(uint32_t imageIndex);
	void present(uint32_t imageIndex);
};
