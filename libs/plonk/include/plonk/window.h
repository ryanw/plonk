#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_X11
#include "event.h"
#include <GLFW/glfw3.h>
#include <functional>

class Window {
public:
	Window(int width, int height);
	~Window();
	GLFWwindow *inner;

	void run(std::function<void(Event)> callback);
	int width();
	int height();
};
