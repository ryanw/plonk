#include "include/plonk/context.h"
#include <GLFW/glfw3.h>
#include <iostream>

void Context::attachWindow(Window &window) {
	std::cout << "Attaching window\n";

	glfwMakeContextCurrent(window.inner);

	while (!glfwWindowShouldClose(window.inner)) {
		glfwSwapBuffers(window.inner);
		glfwPollEvents();
	}

	glfwTerminate();

	std::cout << "Window detached\n";
}
