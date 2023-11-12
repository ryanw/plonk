#pragma once

#include "context.h"
#include "camera.h"
#include <chrono>

class Renderer {
public:
	Renderer(Context &ctx);
	~Renderer();
	void draw(Camera &camera);

private:
	Context &ctx;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::chrono::time_point<std::chrono::high_resolution_clock> startedAt;

	void handleResize();
	void createRenderPass();
	void createPipeline();
	void createCommandPool();
	void createCommandBuffer();
	void recordCommands(Camera &camera);
	void present();
};
