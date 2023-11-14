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
	auto transform = Matrix4::identity();
	auto look_at = Matrix4::look_at(direction);
	transform *= look_at;
	transform *= Matrix4::from_rotation(x, y, 0.0);
	transform *= look_at.inverse().value();
	direction = transform * direction;
}

void Camera::translate(float x, float y, float z) {
	auto transform = Matrix4::identity();
	auto look_at = Matrix4::look_at(direction);
	transform *= look_at;
	transform *= Matrix4::from_translation(x, y, z);
	transform *= look_at.inverse().value();
	position = transform * position;
}
