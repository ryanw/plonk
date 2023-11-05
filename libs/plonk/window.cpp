#include "include/plonk/window.h"
#include <chrono>
#include <iostream>
#include <thread>

Window::Window(int width, int height) {
	if (!glfwInit()) {
		std::cout << "Error initialising glfw\n";
		return;
	}

	glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	inner = glfwCreateWindow(1024, 768, "Plonk", NULL, NULL);

	if (!inner) {
		std::cout << "Error creating window\n";
		glfwTerminate();
		return;
	}

	std::cout << "Finished Plonk\n";
}

void Window::run(std::function<void(Event)> callback) {
	while (!glfwWindowShouldClose(inner)) {
		glfwPollEvents();
		Event event = DrawEvent{.dt = 1.0 / 60.0};
		callback(event);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	glfwTerminate();
}

Window::~Window() { glfwTerminate(); }
