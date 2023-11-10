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
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	// FIXME make private
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

	Context();
	~Context();
	void attachWindow(Window &window);
	VkShaderModule loadShader(const std::string &filename);
	void destroyShader(VkShaderModule shader);
	float width() { return extent.width; };
	float height() { return extent.height; };
	VkExtent2D size() { return extent; };
	VkFormat format() { return surfaceFormat.format; };
	void updateSwapchain();
	bool needsResize();
	uint32_t swapchainImageCount() { return swapchainImages.size(); };
	Frame aquireFrame();
	VkSwapchainKHR getSwapchain() { return swapchain; };
	VkImage getSwapchainImage(int index) { return swapchainImages[index]; };
	VkImageView getSwapchainImageView(int index) { return swapchainImageViews[index]; };
	std::optional<uint32_t> getGraphicsQueueFamilyIndex() { return graphicsQueueFamilyIndex; };
	std::optional<uint32_t> getPresentQueueFamilyIndex() { return presentQueueFamilyIndex; };
	void submit(VkCommandBuffer &commandBuffer);
	void present();
	void beginRenderPass(FrameIndex index);
	void presentFrame(Frame &frame);
	VkPipeline createGraphicsPipeline(VkGraphicsPipelineCreateInfo *pipelineInfo);
	void bindPipeline(VkPipeline &pipeline);

private:
	friend class Frame;

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;
	Window* window = nullptr;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence inFlightFence = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::optional<uint32_t> graphicsQueueFamilyIndex;
	std::optional<uint32_t> presentQueueFamilyIndex;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	void initVulkan();
	void rebuildSwapchain();
	void resizeSwapchain(uint32_t width, uint32_t height);
	void rebuildImageViews();
	void createSyncObjects();
	auto findGraphicsQueue() -> std::optional<uint32_t>;
	auto findPresentQueue() -> std::optional<uint32_t>;
	void rebuildFramebuffers();
	void createRenderPass();
	void createCommandPool();
	VkCommandBuffer createCommandBuffer();
};
