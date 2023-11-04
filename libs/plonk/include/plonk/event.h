#pragma once

#include <variant>

struct DrawEvent {
	double dt;
};
struct MouseEvent {};
struct KeyEvent {};

using Event = std::variant<DrawEvent, MouseEvent, KeyEvent>;
