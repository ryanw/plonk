#pragma once

#include "context.h"

class Renderer {
public:
	Renderer(Context *ctx);
	~Renderer();

private:
	Context *ctx;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;

	void createRenderPass();
	void createPipeline();
	void createFramebuffers();
};
