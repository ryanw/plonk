#pragma once

#include "window.h"
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Context {
public:
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
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
	VkSwapchainKHR getSwapchain() { return swapchain; };
	VkImage getSwapchainImage(int index) { return swapchainImages[index]; };
	VkImageView getSwapchainImageView(int index) { return swapchainImageViews[index]; };
	std::optional<uint32_t> getGraphicsQueueFamilyIndex() { return graphicsQueueFamilyIndex; };
	std::optional<uint32_t> getPresentQueueFamilyIndex() { return presentQueueFamilyIndex; };

private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;
	Window* window;
	std::optional<uint32_t> graphicsQueueFamilyIndex;
	std::optional<uint32_t> presentQueueFamilyIndex;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	void initVulkan();
	void rebuildSwapchain();
	void resizeSwapchain(uint32_t width, uint32_t height);
	void rebuildImageViews();
	auto findPresentQueue() -> std::optional<uint32_t>;
};
