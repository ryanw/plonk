#include "include/plonk/window.h"
#include <chrono>
#include <iostream>
#include <thread>

void Window::glfwKeyCallback(GLFWwindow* inner, int key, int scancode, int action, int mods) {
	Window *window = static_cast<Window*>(glfwGetWindowUserPointer(inner));
	switch (action) {
		case GLFW_PRESS:
			window->keyPressCallback((Key)key, mods);
			break;
		case GLFW_RELEASE:
			window->keyReleaseCallback((Key)key, mods);
			break;
		case GLFW_REPEAT:
			window->keyRepeatCallback((Key)key, mods);
			break;
	}
}

void Window::glfwMouseCallback(GLFWwindow* inner, double x, double y) {
	Window *window = static_cast<Window*>(glfwGetWindowUserPointer(inner));
	window->mouseMoveCallback(x, y);
}

Window::Window(int width, int height) {
	if (!glfwInit()) {
		std::cout << "Error initialising glfw\n";
		return;
	}


	glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	std::cout << "Creating window\n";
	inner = glfwCreateWindow(1024, 768, "Plonk", NULL, NULL);
	glfwSetWindowUserPointer(inner, this);

	if (!inner) {
		std::cout << "Error creating window\n";
		glfwTerminate();
		return;
	}


	glfwSetKeyCallback(inner, glfwKeyCallback);
	glfwSetCursorPosCallback(inner, glfwMouseCallback);

	std::cout << "Window created\n";
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

void Window::onMouseMove(std::function<void(Point2)> callback) {
	mouseMoveCallbacks.push_back(callback);
}

void Window::onKeyPress(Key key, std::function<void(void)> callback) {
	if (!keyPressCallbacks.contains(key)) {
		keyPressCallbacks[key] = std::vector<std::function<void(void)>>();
	}
	keyPressCallbacks[key].push_back(callback);
}

void Window::onKeyRelease(Key key, std::function<void(void)> callback) {
	if (!keyReleaseCallbacks.contains(key)) {
		keyReleaseCallbacks[key] = std::vector<std::function<void(void)>>();
	}
	keyReleaseCallbacks[key].push_back(callback);
}

bool Window::isOpen() {
	return !glfwWindowShouldClose(inner);
}

bool Window::poll() {
	glfwPollEvents();
	return isOpen();
}

bool Window::isKeyHeld(Key key) {
	return heldKeys.contains(key);
}

void Window::keyPressCallback(Key key, int mods) {
	heldKeys.insert(key);
	if (keyPressCallbacks.contains(key)) {
		auto callbacks = keyPressCallbacks[key];
		for (auto &callback : callbacks) {
			callback();
		}
	}
}

void Window::keyReleaseCallback(Key key, int mods) {
	heldKeys.erase(key);
	if (keyReleaseCallbacks.contains(key)) {
		auto callbacks = keyReleaseCallbacks[key];
		for (auto &callback : callbacks) {
			callback();
		}
	}
}

void Window::keyRepeatCallback(Key key, int mods) {
}
void Window::mouseMoveCallback(double x, double y) {
	mouseX = x;
	mouseY = y;
	Point2 p(x, y);
	for (auto &callback : mouseMoveCallbacks) {
		callback(p);
	}
}
Point2 Window::mousePosition() {
	return Point2(mouseX, mouseY);
}

void Window::grabMouse() {
	printf("Grabbing mouse\n");
	glfwSetInputMode(inner, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::releaseMouse() {
	printf("Releasing mouse\n");
	glfwSetInputMode(inner, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::isMouseGrabbed() {
	auto mode = glfwGetInputMode(inner, GLFW_CURSOR);
	return mode == GLFW_CURSOR_DISABLED;
}

Window::~Window() { glfwTerminate(); }
