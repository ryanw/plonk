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
	VkShaderModule vert_shader;
	VkShaderModule frag_shader;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	std::chrono::time_point<std::chrono::high_resolution_clock> started_at;

	void handle_resize();
	void create_render_pass();
	void create_pipeline();
	void create_command_pool();
	void create_command_buffer();
	void record_commands(Camera &camera);
	void present();
};
