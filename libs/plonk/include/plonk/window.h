#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_X11
#include "event.h"
#include "keys.h"
#include "math.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Window {
public:
	Window(int width, int height);
	~Window();
	GLFWwindow *inner;

	void run(std::function<void(Event)> callback);
	int width();
	int height();
	bool isOpen();
	bool poll();
	bool isKeyHeld(Key key);
	void grabMouse();
	void releaseMouse();
	bool isMouseGrabbed();
	Point2 mousePosition();
	void onKeyPress(Key key, std::function<void(void)> callback);
	void onKeyRelease(Key key, std::function<void(void)> callback);
	void onMouseMove(std::function<void(Point2)> callback);

private:
	double mouseX;
	double mouseY;
	std::unordered_set<Key> heldKeys;
	std::unordered_map<Key, std::vector<std::function<void()>>> keyPressCallbacks;
	std::unordered_map<Key, std::vector<std::function<void()>>> keyReleaseCallbacks;
	std::vector<std::function<void(Point2)>> mouseMoveCallbacks;

	static void glfwKeyCallback(GLFWwindow* inner, int key, int scancode, int action, int mods);
	static void glfwMouseCallback(GLFWwindow* inner, double x, double y);
	void keyPressCallback(Key key, int mods);
	void keyReleaseCallback(Key key, int mods);
	void keyRepeatCallback(Key key, int mods);
	void mouseMoveCallback(double x, double y);
};
