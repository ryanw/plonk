#include "include/plonk/renderer.h"
#include <iostream>
#include <vulkan/vulkan.h>

Renderer::Renderer() {
	std::cout << "Creating Renderer\n";
	initVulkan();
}

void Renderer::initVulkan() {
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello, Plonker",
		.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
		.pEngineName = "Plonk",
		.engineVersion = VK_MAKE_VERSION(0, 0, 1),
		.apiVersion = VK_MAKE_VERSION(0, 0, 1),
	};
}
