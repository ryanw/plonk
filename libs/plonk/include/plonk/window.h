#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_X11
#include "event.h"
#include "keys.h"
#include "math.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_set>

class Window {
public:
	Window(int width, int height);
	~Window();
	GLFWwindow *inner;

	void run(std::function<void(Event)> callback);
	int width();
	int height();
	bool is_open();
	bool poll();
	bool is_key_held(Key key);
	void grab_mouse();
	void release_mouse();
	bool is_mouse_grabbed();
	Point2 mouse_position();
	void on_key_press(Key key, std::function<void(void)> callback);
	void on_key_release(Key key, std::function<void(void)> callback);
	void on_mouse_move(std::function<void(Point2)> callback);

private:
	double mouse_x;
	double mouse_y;
	std::unordered_set<Key> held_keys;
	std::unordered_map<Key, std::vector<std::function<void()>>> key_press_callbacks;
	std::unordered_map<Key, std::vector<std::function<void()>>> key_release_callbacks;
	std::vector<std::function<void(Point2)>> mouse_move_callbacks;

	static void glfw_key_callback(GLFWwindow* inner, int key, int scancode, int action, int mods);
	static void glfw_mouse_callback(GLFWwindow* inner, double x, double y);
	void key_press_callback(Key key, int mods);
	void key_release_callback(Key key, int mods);
	void key_repeat_callback(Key key, int mods);
	void mouse_move_callback(double x, double y);
};
