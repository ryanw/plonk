#include "include/plonk/camera.h"
#include <iostream>

Camera::Camera() {}

Camera::~Camera() {}

void Camera::set_position(Point3 position) {
	this->position = position;
}

void Camera::set_direction(Vector3 direction) {
	this->direction = direction.normalize();
}

void Camera::rotate(float x, float y) {
	auto angle = y;
	auto new_x = direction.coords[0] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	auto new_y = direction.coords[0] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(new_x, direction.coords[1], new_y).normalize();

	angle = x;
	new_x = direction.coords[1] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	new_y = direction.coords[1] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(direction.coords[0], new_x, new_y).normalize();

}

void Camera::translate(float x, float y, float z) {
	position = position + direction * z;
	//position.coords[0] += x * direction.coords[0];
	//position.coords[1] += y * direction.coords[1];
	//position.coords[2] += z * direction.coords[2];
}
