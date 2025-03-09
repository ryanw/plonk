#include "include/plonk/context.h"
#include "include/plonk/frame.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

auto choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) -> VkSurfaceFormatKHR {
	for (const auto &available_format : available_formats) {
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_format;
		}
	}

	return available_formats[0];
}

auto load_file(const std::string &filename) -> std::vector<char> {
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

Context::Context() {}

void Context::create_command_pool() {
	printf("Creating command pool\n");
	auto queue_family_index = get_graphics_queue_family_index();
	if (!queue_family_index.has_value()) {
		throw std::runtime_error("No queue family index defined");
	}
	VkCommandPoolCreateInfo pool_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family_index.value(),
	};

	if (VK_SUCCESS != vkCreateCommandPool(device, &pool_info, nullptr, &command_pool)) {
		throw std::runtime_error("Failed to create Command Pool");
	}
}

VkCommandBuffer Context::create_command_buffer() {
	VkCommandBufferAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	if (VK_SUCCESS != vkAllocateCommandBuffers(device, &alloc_info, &command_buffer)) {
		throw std::runtime_error("Failed to allocate command buffer");
	}

	return command_buffer;
}

VkPipeline Context::create_graphics_pipeline(VkGraphicsPipelineCreateInfo *pipeline_info) {
	VkPipeline pipeline;
	if (!pipeline_info->renderPass) {
		pipeline_info->renderPass = render_pass;
	}
	if (VK_SUCCESS != vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, pipeline_info, nullptr, &pipeline)) {
		throw std::runtime_error("Failed to create Pipeline");
	}

	return pipeline;
}

void Context::bind_pipeline(VkPipeline &pipeline) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

/**
 * Load a SPIR-V shader from disk
 *
 * @param filename Path to the compiled shader
 * @return The Vulkan shader module
 */
auto Context::load_shader(const std::string &filename) -> VkShaderModule {
	std::cout << "Opening shader: " << filename << "\n";
	auto code = load_file(filename);

	VkShaderModuleCreateInfo create_info{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t *>(code.data()),
	};

	VkShaderModule shader;
	if (VK_SUCCESS != vkCreateShaderModule(device, &create_info, nullptr, &shader)) {
		throw std::runtime_error("Failed to create shader module");
	}

	std::cout << "Shader loaded:" << filename << "\n";
	return shader;
}

void Context::destroy_shader(VkShaderModule shader) {
	vkDestroyShaderModule(device, shader, nullptr);
}

void Context::attach_window(std::shared_ptr<Window> window) {
	std::cout << "Attaching window\n";
	this->window = window;
	init_vulkan();

	VkResult result = glfwCreateWindowSurface(instance, window->inner, nullptr, &surface);
	if (VK_SUCCESS != result) {
		throw std::runtime_error("Failed to create window surface");
	}

	present_queue_family_index = find_present_queue();
	if (!present_queue_family_index.has_value()) {
		throw std::runtime_error("Couldn't find a valid present queue family");
	}
	vkGetDeviceQueue(device, present_queue_family_index.value(), 0, &present_queue);

	resize_swapchain(window->width(), window->height());
	create_render_pass();
	create_command_pool();
	create_command_buffer();
	rebuild_image_views();
	rebuild_framebuffers();
}

bool Context::needs_resize() {
	return (extent.width != window->width() || extent.height != window->height());
}

void Context::update_swapchain() {
	if (needs_resize()) {
		rebuild_swapchain();
	}
}

void Context::rebuild_swapchain() {
	if (!window) {
		throw std::runtime_error("Can't create swapchain without an attached window");
	}
	resize_swapchain(window->width(), window->height());
	rebuild_image_views();
	rebuild_framebuffers();
}

auto Context::find_present_queue() -> std::optional<uint32_t> {
	std::optional<uint32_t> result;

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);

	std::vector<VkQueueFamilyProperties> families(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, families.data());

	int i = 0;
	for (const auto &family : families) {
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
		if (present_support)
			return i;
		i++;
	}

	return result;
}

auto Context::find_graphics_queue() -> std::optional<uint32_t> {
	std::optional<uint32_t> result;

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, nullptr);

	std::vector<VkQueueFamilyProperties> families(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, families.data());

	int i = 0;
	for (const auto &family : families) {
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			return i;
		}
		i++;
	}

	return 0;
}

void Context::init_vulkan() {
	std::cout << "Initialising Vulkan\n";
	uint32_t extension_count = 0;
	auto extension_names = glfwGetRequiredInstanceExtensions(&extension_count);


	const std::vector<const char *> validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};

	VkApplicationInfo app_info{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello, Plonker",
		.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
		.pEngineName = "Plonk",
		.engineVersion = VK_MAKE_VERSION(0, 0, 1),
		.apiVersion = VK_API_VERSION_1_4,
	};

	VkInstanceCreateInfo instance_create_info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
		.ppEnabledLayerNames = validation_layers.data(),
		.enabledExtensionCount = extension_count,
		.ppEnabledExtensionNames = extension_names,
	};

	if (VK_SUCCESS != vkCreateInstance(&instance_create_info, nullptr, &instance)) {
		throw std::runtime_error("Failed to create Vulkan instance");
	}
	std::cout << "Vulkan instance created\n";

	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
	printf("Found %d device(s)\n", device_count);

	if (device_count == 0) {
		throw std::runtime_error("Couldn't find a GPU");
	}

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
	physical_device = devices[0];

	graphics_queue_family_index = find_graphics_queue();
	if (!graphics_queue_family_index.has_value()) {
		throw std::runtime_error("Couldn't find a valid graphics queue family");
	}

	std::cout << "Queue family found\n";

	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_create_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphics_queue_family_index.value(),
		.queueCount = 1,
		.pQueuePriorities = &queue_priority,
	};

	const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkPhysicalDeviceFeatures device_features{};
	VkDeviceCreateInfo device_create_info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_create_info,
		.enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
		.ppEnabledLayerNames = validation_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
		.ppEnabledExtensionNames = device_extensions.data(),
		.pEnabledFeatures = &device_features,
	};

	if (VK_SUCCESS != vkCreateDevice(physical_device, &device_create_info, nullptr, &device)) {
		throw std::runtime_error("Failed to create logical device");
	}
	std::cout << "Logical device created\n";

	vkGetDeviceQueue(device, graphics_queue_family_index.value(), 0, &graphics_queue);

	create_sync_objects();
}

void Context::resize_swapchain(uint32_t width, uint32_t height) {
	if (swapchain) {
		std::cout << "Destroying old Swap Chain\n";
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = nullptr;
	}
	std::cout << "Creating Swap Chain\n";
	surface_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	};

	extent = {
		.width = width,
		.height = height,
	};

	VkSurfaceCapabilitiesKHR caps;
	std::cout << "Checking device capabilities\n";
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
	uint32_t image_count = caps.minImageCount;
	printf("Swapchain has %d images\n", image_count);

	VkSwapchainCreateInfoKHR create_info{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = image_count,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = true,
		.oldSwapchain = nullptr,
	};

	if (VK_SUCCESS != vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain)) {
		throw std::runtime_error("Failed to create swapchain");
	}
	std::cout << "Created Swap Chain\n";
}

void Context::rebuild_image_views() {
	for (auto view : swapchain_image_views) {
		vkDestroyImageView(device, view, nullptr);
		swapchain_image_views.clear();
	}

	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
	uint32_t image_count = caps.minImageCount;
	printf("Creating %d Swapchain Image Views\n", image_count);

	vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
	swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

	swapchain_image_views.resize(swapchain_images.size());

	for (int i = 0; i < image_count; i++) {
		VkImageViewCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surface_format.format,
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

		if (VK_SUCCESS != vkCreateImageView(device, &create_info, nullptr, &swapchain_image_views[i])) {
			throw std::runtime_error("Failed to create swapchain Image Views");
		}
	}
}

Frame Context::aquire_frame() {
	vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &in_flight_fence);
	uint32_t index;
	vkAcquireNextImageKHR(device, get_swapchain(), UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &index);
	Frame frame(as_shared_ptr(), index);
	vkResetCommandBuffer(command_buffer, 0);
	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr,
	};
	if (VK_SUCCESS != vkBeginCommandBuffer(command_buffer, &begin_info)) {
		throw std::runtime_error("Failed to start command recording");
	}
	begin_render_pass(index);
	return frame;
}

void Context::begin_render_pass(FrameIndex index) {
	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo render_pass_info{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass,
		.framebuffer = framebuffers[index],
		.renderArea =
			{
				.offset = {0, 0},
				.extent = size(),
			},
		.clearValueCount = 1,
		.pClearValues = &clear_color,
	};
	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Context::submit(VkCommandBuffer &command_buffer) {
	VkSemaphore wait_semaphores[] = {image_available_semaphore};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {render_finished_semaphore};
	VkSubmitInfo submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores,
	};

	if (VK_SUCCESS != vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence)) {
		throw std::runtime_error("Failed to submit queue");
	}
}
void Context::present_frame(Frame &frame) {
	vkCmdEndRenderPass(command_buffer);
	vkEndCommandBuffer(command_buffer);
	submit(command_buffer);

	VkSemaphore wait_semaphores[] = {image_available_semaphore};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {render_finished_semaphore};

	VkSwapchainKHR swapchains[] = {get_swapchain()};
	VkPresentInfoKHR present_info{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &frame.index,
		.pResults = nullptr,
	};
	vkQueuePresentKHR(present_queue, &present_info);
	vkDeviceWaitIdle(device);
}

void Context::create_sync_objects() {
	printf("Creating sync objects\n");

	VkSemaphoreCreateInfo semaphore_info{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fence_info{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	if (VK_SUCCESS != vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphore)) {
		throw std::runtime_error("Failed to create image available semaphore");
	}
	if (VK_SUCCESS != vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphore)) {
		throw std::runtime_error("Failed to create render finished semaphore");
	}
	if (VK_SUCCESS != vkCreateFence(device, &fence_info, nullptr, &in_flight_fence)) {
		throw std::runtime_error("Failed to create in-flight fence");
	}
}

void Context::rebuild_framebuffers() {
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		framebuffers.clear();
	}
	auto count = swapchain_image_count();
	printf("Creating %d Framebuffers\n", count);

	framebuffers.resize(count);

	for (int i = 0; i < count; i++) {
		printf("Framebuffer: %d\n", i);
		auto image_view = get_swapchain_image_view(i);
		if (!image_view) {
			throw std::runtime_error("Missing image view");
		}

		VkFramebufferCreateInfo framebuffer_info{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = render_pass,
			.attachmentCount = 1,
			.pAttachments = &image_view,
			.width = (uint32_t)width(),
			.height = (uint32_t)height(),
			.layers = 1,
		};

		if (VK_SUCCESS != vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i])) {
			throw std::runtime_error("Failed to create Framebuffer");
		}
	}
}

void Context::create_render_pass() {
	std::cout << "Creating Render Pass\n";
	VkAttachmentDescription color_attachment{
		.format = format(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference color_attachment_ref{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
	};

	VkRenderPassCreateInfo render_pass_create_info{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
	};

	if (VK_SUCCESS != vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass)) {
		throw std::runtime_error("Failed to create RenderPass");
	}
	std::cout << "Render pass created\n";
}

Context::~Context() {
	std::cout << "Destroying Plonk Context\n";
	vkDestroyCommandPool(device, command_pool, nullptr);
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(device, render_pass, nullptr);
	vkDestroySemaphore(device, image_available_semaphore, nullptr);
	vkDestroySemaphore(device, render_finished_semaphore, nullptr);
	vkDestroyFence(device, in_flight_fence, nullptr);
	for (auto view : swapchain_image_views) {
		vkDestroyImageView(device, view, nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}
