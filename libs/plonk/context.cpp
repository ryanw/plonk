#include "include/plonk/context.h"
#include "include/plonk/frame.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) -> VkSurfaceFormatKHR {
	for (const auto &availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

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

Context::Context() {
}

void Context::createCommandPool() {
	printf("Creating command pool\n");
	auto queueFamilyIndex = getGraphicsQueueFamilyIndex();
	if (!queueFamilyIndex.has_value()) {
		throw std::runtime_error("No queue family index defined");
	}
	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndex.value(),
	};

	if (VK_SUCCESS != vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)) {
		throw std::runtime_error("Failed to create Command Pool");
	}
}

VkCommandBuffer Context::createCommandBuffer() {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	if (VK_SUCCESS != vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer)) {
		throw std::runtime_error("Failed to allocate command buffer");
	}

	return commandBuffer;
}

VkPipeline Context::createGraphicsPipeline(VkGraphicsPipelineCreateInfo *pipelineInfo) {
	VkPipeline pipeline;
	if (!pipelineInfo->renderPass) {
		pipelineInfo->renderPass = renderPass;
	}
	if (VK_SUCCESS != vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, pipelineInfo, nullptr, &pipeline)) {
		throw std::runtime_error("Failed to create Pipeline");
	}

	return pipeline;
}

void Context::bindPipeline(VkPipeline &pipeline) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

/**
 * Load a SPIR-V shader from disk
 *
 * @param filename Path to the compiled shader
 * @return The Vulkan shader module
 */
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

void Context::attachWindow(Window &window) {
	std::cout << "Attaching window\n";

	this->window = &window;

	initVulkan();

	VkResult result = glfwCreateWindowSurface(instance, window.inner, nullptr, &surface);
	if (VK_SUCCESS != result) {
		throw std::runtime_error("Failed to create window surface");
	}

	presentQueueFamilyIndex = findPresentQueue();
	if (!presentQueueFamilyIndex.has_value()) {
		throw std::runtime_error("Couldn't find a valid present queue family");
	}
	vkGetDeviceQueue(device, presentQueueFamilyIndex.value(), 0, &presentQueue);

	resizeSwapchain(window.width(), window.height());
	createRenderPass();
	createCommandPool();
	createCommandBuffer();
	rebuildImageViews();
	rebuildFramebuffers();
}

bool Context::needsResize() {
	return (extent.width != window->width() || extent.height != window->height());
}

void Context::updateSwapchain() {
	if (needsResize()) {
		rebuildSwapchain();
	}
}

void Context::rebuildSwapchain() {
	if (!window) {
		throw std::runtime_error("Can't create swapchain without an attached window");
	}
	resizeSwapchain(window->width(), window->height());
	rebuildImageViews();
	rebuildFramebuffers();
}

auto Context::findPresentQueue() -> std::optional<uint32_t> {
	std::optional<uint32_t> result;

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

	std::vector<VkQueueFamilyProperties> families(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

	int i = 0;
	for (const auto &family : families) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport)
			return i;
		i++;
	}

	return result;
}

auto Context::findGraphicsQueue() -> std::optional<uint32_t> {
	std::optional<uint32_t> result;

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

	std::vector<VkQueueFamilyProperties> families(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

	int i = 0;
	for (const auto &family : families) {
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			return i;
		}
		i++;
	}

	return 0;
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

	graphicsQueueFamilyIndex = findGraphicsQueue();
	if (!graphicsQueueFamilyIndex.has_value()) {
		throw std::runtime_error("Couldn't find a valid graphics queue family");
	}

	std::cout << "Queue family found\n";

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsQueueFamilyIndex.value(),
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

	vkGetDeviceQueue(device, graphicsQueueFamilyIndex.value(), 0, &graphicsQueue);

	createSyncObjects();
}

void Context::resizeSwapchain(uint32_t width, uint32_t height) {
	if (swapchain) {
		std::cout << "Destroying old Swap Chain\n";
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = nullptr;
	}
	std::cout << "Creating Swap Chain\n";
	surfaceFormat = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	};

	extent = {
		.width = width,
		.height = height,
	};

	VkSurfaceCapabilitiesKHR caps;
	std::cout << "Checking device capabilities\n";
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
	uint32_t imageCount = caps.minImageCount;
	printf("Swapchain has %d images\n", imageCount);

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
		.oldSwapchain = nullptr,
	};

	if (VK_SUCCESS != vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain)) {
		throw std::runtime_error("Failed to create swapchain");
	}
	std::cout << "Created Swap Chain\n";
}

void Context::rebuildImageViews() {
	for (auto view : swapchainImageViews) {
		vkDestroyImageView(device, view, nullptr);
		swapchainImageViews.clear();
	}

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
			.image = swapchainImages[i],
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

Frame Context::aquireFrame() {
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);
	uint32_t index;
	vkAcquireNextImageKHR(device, getSwapchain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &index);
	Frame frame(*this, index);
	vkResetCommandBuffer(commandBuffer, 0);
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr,
	};
	if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
		throw std::runtime_error("Failed to start command recording");
	}
	beginRenderPass(index);
	return frame;
}

void Context::beginRenderPass(FrameIndex index) {
	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer = framebuffers[index],
		.renderArea =
			{
				.offset = {0, 0},
				.extent = size(),
			},
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Context::submit(VkCommandBuffer &commandBuffer) {
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	if (VK_SUCCESS != vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence)) {
		throw std::runtime_error("Failed to submit queue");
	}
}
void Context::presentFrame(Frame &frame) {
	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
	submit(commandBuffer);

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

	VkSwapchainKHR swapchains[] = {getSwapchain()};
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &frame.index,
		.pResults = nullptr,
	};
	vkQueuePresentKHR(presentQueue, &presentInfo);
	vkDeviceWaitIdle(device);
}


void Context::createSyncObjects() {
	printf("Creating sync objects\n");

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	if (VK_SUCCESS != vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore)) {
		throw std::runtime_error("Failed to create image available semaphore");
	}
	if (VK_SUCCESS != vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore)) {
		throw std::runtime_error("Failed to create render finished semaphore");
	}
	if (VK_SUCCESS != vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence)) {
		throw std::runtime_error("Failed to create in-flight fence");
	}
}

void Context::rebuildFramebuffers() {
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		framebuffers.clear();
	}
	auto count = swapchainImageCount();
	printf("Creating %d Framebuffers\n", count);

	framebuffers.resize(count);

	for (int i = 0; i < count; i++) {
		printf("Framebuffer: %d\n", i);
		auto imageView = getSwapchainImageView(i);
		if (!imageView) {
			throw std::runtime_error("Missing image view");
		}

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = &imageView,
			.width = (uint32_t)width(),
			.height = (uint32_t)height(),
			.layers = 1,
		};

		if (VK_SUCCESS != vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i])) {
			throw std::runtime_error("Failed to create Framebuffer");
		}
	}
}

void Context::createRenderPass() {
	std::cout << "Creating Render Pass\n";
	VkAttachmentDescription colorAttachment{
		.format = format(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
	};

	if (VK_SUCCESS != vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass)) {
		throw std::runtime_error("Failed to create RenderPass");
	}
	std::cout << "Render pass created\n";
}

Context::~Context() {
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);
	for (auto view : swapchainImageViews) {
		vkDestroyImageView(device, view, nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}
