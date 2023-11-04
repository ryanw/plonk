#pragma once

#include <GLFW/glfw3.h>

class Window {
  public:
	Window(int width, int height);
	GLFWwindow *inner;
};
