#include "include/plonk/window.h"
#include <iostream>

Window::Window(int width, int height) {
	if (!glfwInit()) {
		std::cout << "Error initialising glfw\n";
		return;
	}

	inner = glfwCreateWindow(1024, 768, "Plonker", NULL, NULL);
	if (!inner) {
		std::cout << "Error creating window\n";
		glfwTerminate();
		return;
	}

	std::cout << "Finished Plonk\n";
}
