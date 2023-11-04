#pragma once

#include "window.h"
#include <vector>
#include <vulkan/vulkan.h>

class Context {
public:
	Context();
	~Context();
	void attachWindow(Window &window);

private:
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue queue;
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
