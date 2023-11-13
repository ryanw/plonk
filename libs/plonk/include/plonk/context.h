#pragma once

#include "window.h"
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

typedef uint32_t FrameIndex;
class Frame;

class Context {
public:
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	// FIXME make private
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;

	Context();
	~Context();
	void attach_window(Window &window);
	VkShaderModule load_shader(const std::string &filename);
	void destroy_shader(VkShaderModule shader);
	float width() { return extent.width; };
	float height() { return extent.height; };
	VkExtent2D size() { return extent; };
	VkFormat format() { return surface_format.format; };
	void update_swapchain();
	bool needs_resize();
	uint32_t swapchain_image_count() { return swapchain_images.size(); };
	Frame aquire_frame();
	VkSwapchainKHR get_swapchain() { return swapchain; };
	VkImage get_swapchain_image(int index) { return swapchain_images[index]; };
	VkImageView get_swapchain_image_view(int index) { return swapchain_image_views[index]; };
	std::optional<uint32_t> get_graphics_queue_family_index() { return graphics_queue_family_index; };
	std::optional<uint32_t> get_present_queue_family_index() { return present_queue_family_index; };
	void submit(VkCommandBuffer &command_buffer);
	void present();
	void begin_render_pass(FrameIndex index);
	void present_frame(Frame &frame);
	VkPipeline create_graphics_pipeline(VkGraphicsPipelineCreateInfo *pipeline_info);
	void bind_pipeline(VkPipeline &pipeline);

private:
	friend class Frame;

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
	Window* window = nullptr;
	VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
	VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
	VkFence in_flight_fence = VK_NULL_HANDLE;
	VkRenderPass render_pass = VK_NULL_HANDLE;
	std::optional<uint32_t> graphics_queue_family_index;
	std::optional<uint32_t> present_queue_family_index;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool command_pool = VK_NULL_HANDLE;

	void init_vulkan();
	void rebuild_swapchain();
	void resize_swapchain(uint32_t width, uint32_t height);
	void rebuild_image_views();
	void create_sync_objects();
	auto find_graphics_queue() -> std::optional<uint32_t>;
	auto find_present_queue() -> std::optional<uint32_t>;
	void rebuild_framebuffers();
	void create_render_pass();
	void create_command_pool();
	VkCommandBuffer create_command_buffer();
};
