#include <iostream>
#include <plonk/plonk.h>

int main(int, char **) {
	std::cout << "Starting Plonk...\n";

	auto window = std::make_shared<Window>(1920, 1080);
	auto ctx = Context::create();
	ctx->attach_window(window);

	Camera camera;
	camera.set_position(Point3(0.0, -1.0, -12.0));
	Renderer renderer(ctx);

	auto speed = 60.0;

	auto last_frame = std::chrono::high_resolution_clock::now();

	window->on_key_press(Key::SPACE, [&]() {
		if (window->is_mouse_grabbed()) {
			window->release_mouse();
		}
		else {
			window->grab_mouse();
		}
	});

	Point2 prev_mouse_pos;
	window->on_mouse_move([&](auto position) {
		auto d = prev_mouse_pos - position;
		float mouse_speed = 800.0;
		if (window->is_mouse_grabbed()) {
			camera.rotate(d.y() / mouse_speed, d.x() / -mouse_speed);
		}
		prev_mouse_pos = position;
	});

	while (window->poll()) {
		auto now = std::chrono::high_resolution_clock::now();
		double dt = (now - last_frame).count() / 1000000000.00;
		last_frame = now;

		if (window->is_key_held(Key::W)) {
			camera.translate(0.0, 0.0, speed * dt);
		}
		if (window->is_key_held(Key::S)) {
			camera.translate(0.0, 0.0, -speed * dt);
		}
		if (window->is_key_held(Key::A)) {
			camera.translate(-speed * dt, 0.0, 0.0);
		}
		if (window->is_key_held(Key::D)) {
			camera.translate(speed * dt, 0.0, 0.0);
		}
		if (window->is_key_held(Key::E)) {
			camera.translate(0.0, -speed * dt, 0.0);
		}
		if (window->is_key_held(Key::Q)) {
			camera.translate(0.0, speed * dt, 0.0);
		}
		
		renderer.draw(camera);
	}

	std::cout << "Finished Plonk\n";
	return 0;
}
