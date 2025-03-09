#include "include/plonk/window.h"
#include <chrono>
#include <iostream>
#include <thread>

void Window::glfw_key_callback(GLFWwindow* inner, int key, int scancode, int action, int mods) {
	Window *window = static_cast<Window*>(glfwGetWindowUserPointer(inner));
	switch (action) {
		case GLFW_PRESS:
			window->key_press_callback((Key)key, mods);
			break;
		case GLFW_RELEASE:
			window->key_release_callback((Key)key, mods);
			break;
		case GLFW_REPEAT:
			window->key_repeat_callback((Key)key, mods);
			break;
	}
}

void Window::glfw_mouse_callback(GLFWwindow* inner, double x, double y) {
	Window *window = static_cast<Window*>(glfwGetWindowUserPointer(inner));
	window->mouse_move_callback(x, y);
}

Window::Window(int width, int height) {
	if (!glfwInit()) {
		std::cout << "Error initialising glfw\n";
		return;
	}


	glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	inner = glfwCreateWindow(1024, 768, "Plonk", NULL, NULL);
	glfwSetWindowUserPointer(inner, this);

	if (!inner) {
		std::cout << "Error creating window\n";
		glfwTerminate();
		return;
	}


	glfwSetKeyCallback(inner, glfw_key_callback);
	glfwSetCursorPosCallback(inner, glfw_mouse_callback);

	std::cout << "Finished Plonk\n";
}

int Window::width() {
	int width;
	glfwGetWindowSize(inner, &width, nullptr);
	return width;
}

int Window::height() {
	int height;
	glfwGetWindowSize(inner, nullptr, &height);
	return height;
}

void Window::on_mouse_move(std::function<void(Point2)> callback) {
	mouse_move_callbacks.push_back(callback);
}

void Window::on_key_press(Key key, std::function<void(void)> callback) {
	if (!key_press_callbacks.contains(key)) {
		key_press_callbacks[key] = std::vector<std::function<void(void)>>();
	}
	key_press_callbacks[key].push_back(callback);
}

void Window::on_key_release(Key key, std::function<void(void)> callback) {
	if (!key_release_callbacks.contains(key)) {
		key_release_callbacks[key] = std::vector<std::function<void(void)>>();
	}
	key_release_callbacks[key].push_back(callback);
}

bool Window::is_open() {
	return !glfwWindowShouldClose(inner);
}

bool Window::poll() {
	glfwPollEvents();
	return is_open();
}

bool Window::is_key_held(Key key) {
	return held_keys.contains(key);
}

void Window::key_press_callback(Key key, int mods) {
	held_keys.insert(key);
	if (key_press_callbacks.contains(key)) {
		auto callbacks = key_press_callbacks[key];
		for (auto &callback : callbacks) {
			callback();
		}
	}
}

void Window::key_release_callback(Key key, int mods) {
	held_keys.erase(key);
	if (key_release_callbacks.contains(key)) {
		auto callbacks = key_release_callbacks[key];
		for (auto &callback : callbacks) {
			callback();
		}
	}
}

void Window::key_repeat_callback(Key key, int mods) {
}
void Window::mouse_move_callback(double x, double y) {
	mouse_x = x;
	mouse_y = y;
	Point2 p(x, y);
	for (auto &callback : mouse_move_callbacks) {
		callback(p);
	}
}
Point2 Window::mouse_position() {
	return Point2(mouse_x, mouse_y);
}

void Window::grab_mouse() {
	printf("Grabbing mouse\n");
	glfwSetInputMode(inner, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::release_mouse() {
	printf("Releasing mouse\n");
	glfwSetInputMode(inner, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::is_mouse_grabbed() {
	auto mode = glfwGetInputMode(inner, GLFW_CURSOR);
	return mode == GLFW_CURSOR_DISABLED;
}

Window::~Window() {
	std::cout << "Terminating GLFW window\n";
	glfwTerminate();
}
