#include "include/plonk/context.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

auto loadFile(const std::string &filename) -> std::vector<char> {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open shader");
	}

	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();

	return buffer;
}

auto Context::loadShader(const std::string &filename) -> VkShaderModule {
	std::cout << "Opening shader: " << filename << "\n";
	auto code = loadFile(filename);

	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t *>(code.data()),
	};

	VkShaderModule shader;
	if (VK_SUCCESS != vkCreateShaderModule(device, &createInfo, nullptr, &shader)) {
		throw std::runtime_error("Failed to create shader module");
	}

	std::cout << "Shader loaded:" << filename << "\n";
	return shader;
}

void Context::destroyShader(VkShaderModule shader) { vkDestroyShaderModule(device, shader, nullptr); }

auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) -> VkSurfaceFormatKHR {
	for (const auto &availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

auto findQueueFamilies(VkPhysicalDevice &device) -> std::optional<uint32_t> {
	std::optional<uint32_t> result;

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

	std::vector<VkQueueFamilyProperties> families(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

	int i = 0;
	for (const auto &family : families) {
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			result = i;
			break;
		}
		i++;
	}

	return result;
}

void Context::attachWindow(Window &window) {
	std::cout << "Attaching window\n";

	VkResult result = glfwCreateWindowSurface(instance, window.inner, nullptr, &surface);
	if (VK_SUCCESS != result) {
		throw std::runtime_error("Failed to create window surface");
	}

	createSwapchain();
	createImageViews();
}

void Context::initVulkan() {
	uint32_t extensionCount = 0;
	auto extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);

	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello, Plonker",
		.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
		.pEngineName = "Plonk",
		.engineVersion = VK_MAKE_VERSION(0, 0, 1),
		.apiVersion = VK_MAKE_VERSION(0, 0, 1),
	};

	VkInstanceCreateInfo instanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = extensionCount,
		.ppEnabledExtensionNames = extensionNames,
	};

	if (VK_SUCCESS != vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
		throw std::runtime_error("Failed to create Vulkan instance");
	}
	std::cout << "Vulkan instance created\n";

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	printf("Found %d device(s)\n", deviceCount);

	if (deviceCount == 0) {
		throw std::runtime_error("Couldn't find a GPU");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	physicalDevice = devices[0];

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	auto family = findQueueFamilies(physicalDevice);
	if (!family.has_value()) {
		throw std::runtime_error("Couldn't find a valid queue family");
	}

	std::cout << "Queue family found\n";

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = family.value(),
		.queueCount = 1,
		.pQueuePriorities = &queuePriority,
	};

	const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo deviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	if (VK_SUCCESS != vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
		throw std::runtime_error("Failed to create logical device");
	}
	std::cout << "Logical device created\n";

	vkGetDeviceQueue(device, family.value(), 0, &queue);
}

void Context::createSwapchain() {
	std::cout << "Creating Swap Chain\n";
	surfaceFormat = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	};

	extent = {
		.width = 1024,
		.height = 768,
	};

	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
	uint32_t imageCount = caps.minImageCount;

	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = true,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	if (VK_SUCCESS != vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain)) {
		throw std::runtime_error("Failed to create swapchain");
	}
}

void Context::createImageViews() {
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
	uint32_t imageCount = caps.minImageCount;
	printf("Creating %d Swapchain Image Views\n", imageCount);

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	swapchainImageViews.resize(swapchainImages.size());

	for (int i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchainImages[0],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surfaceFormat.format,
			.components =
				{
					.r = VK_COMPONENT_SWIZZLE_R,
					.g = VK_COMPONENT_SWIZZLE_G,
					.b = VK_COMPONENT_SWIZZLE_B,
					.a = VK_COMPONENT_SWIZZLE_A,
				},
			.subresourceRange =
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
		};

		if (VK_SUCCESS != vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i])) {
			throw std::runtime_error("Failed to create swapchain Image Views");
		}
	}
}

Context::Context() { initVulkan(); }

Context::~Context() {
	for (auto view : swapchainImageViews) {
		vkDestroyImageView(device, view, nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}
