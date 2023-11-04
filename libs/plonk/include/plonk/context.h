#pragma once

#include "window.h"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Context {
public:
	VkDevice device;
	VkQueue queue;
	Context();
	~Context();
	void attachWindow(Window &window);
	VkShaderModule loadShader(const std::string &filename);
	void destroyShader(VkShaderModule shader);
	float width() { return extent.width; };
	float height() { return extent.height; };
	VkExtent2D size() { return extent; };
	VkFormat format() { return surfaceFormat.format; };
	uint32_t swapchainImageCount() { return swapchainImageViews.size(); };
	VkImage getSwapchainImage(int index) { return swapchainImages[index]; };
	VkImageView getSwapchainImageView(int index) { return swapchainImageViews[index]; };

private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	void initVulkan();
	void createSwapchain();
	void createImageViews();
};
