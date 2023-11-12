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

	Camera camera;
	camera.setPosition(Point3(0.0, -1.0, -12.0));
	Renderer renderer(ctx);

	auto speed = 60.0;

	auto lastFrame = std::chrono::high_resolution_clock::now();

	window.onKeyPress(Key::SPACE, [&]() {
		if (window.isMouseGrabbed()) {
			window.releaseMouse();
		}
		else {
			window.grabMouse();
		}
	});

	Point2 prevMousePos;
	window.onMouseMove([&](auto position) {
		auto d = prevMousePos - position;
		if (window.isMouseGrabbed()) {
			camera.rotate(d.y() / 500.0, d.x() / 500.0);
		}
		prevMousePos = position;
	});

	while (window.poll()) {
		auto now = std::chrono::high_resolution_clock::now();
		double dt = (now - lastFrame).count() / 1000000000.00;
		lastFrame = now;

		if (window.isKeyHeld(Key::W)) {
			camera.translate(0.0, 0.0, speed * dt);
		}
		if (window.isKeyHeld(Key::S)) {
			camera.translate(0.0, 0.0, -speed * dt);
		}
		if (window.isKeyHeld(Key::A)) {
			camera.translate(-speed * dt, 0.0, 0.0);
		}
		if (window.isKeyHeld(Key::D)) {
			camera.translate(speed * dt, 0.0, 0.0);
		}
		if (window.isKeyHeld(Key::E)) {
			camera.translate(0.0, -speed * dt, 0.0);
		}
		if (window.isKeyHeld(Key::Q)) {
			camera.translate(0.0, speed * dt, 0.0);
		}
		renderer.draw(camera);
	}

	std::cout << "Finished Plonk\n";
	return 0;
}
