#include <iostream>
#include <plonk/plonk.h>

template <class... Ts> struct overload : Ts... {
	using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

int main(int, char **) {
	std::cout << "Starting Plonk...\n";

	Window window(1920, 1080);

	Context ctx;
	ctx.attachWindow(window);

	Renderer renderer(ctx);

	window.run([&renderer](Event event) {
		std::visit(
			overload{
				[&renderer](DrawEvent &event) { renderer.draw(); },
				[](MouseEvent &event) { std::cout << "MOUSE!\n"; },
				[](KeyEvent &event) { std::cout << "KEY!\n"; },
			},
			event
		);
	});

	std::cout << "Finished Plonk\n";
	return 0;
}
