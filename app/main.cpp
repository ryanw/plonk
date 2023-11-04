#include <iostream>
#include <plonk/plonk.h>

int main(int, char **) {
	std::cout << "Starting Plonk...\n";

	Window window(1920, 1080);

	Context ctx;
	ctx.attachWindow(window);

	Renderer renderer;

	std::cout << "Finished Plonk\n";
	return 0;
}
