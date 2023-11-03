#include <iostream>
#include <plonk/plonk.h>
#include <GLFW/glfw3.h>

int main(int, char **) {
	std::cout << "Starting Plonk...\n";

	if (!glfwInit()) {
		std::cout << "Error initialising glfw\n";
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Plonker", NULL, NULL);
	if (!window) {
		std::cout << "Error creating window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	std::cout << "Finished Plonk\n";
	return 0;
}
