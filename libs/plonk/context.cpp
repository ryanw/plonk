#include "include/plonk/context.h"
#include <iostream>
#include <GLFW/glfw3.h>

void Context::attachWindow(Window& window) {
	std::cout << "Attaching window\n";

	glfwMakeContextCurrent(window.inner);

	while (!glfwWindowShouldClose(window.inner)) {
		glfwSwapBuffers(window.inner);
		glfwPollEvents();
	}

	glfwTerminate();

	std::cout << "Window detached\n";
}
