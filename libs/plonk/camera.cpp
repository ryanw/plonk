#include "include/plonk/camera.h"
#include <iostream>

Camera::Camera() {}

Camera::~Camera() {}

void Camera::setPosition(Point3 position) {
	this->position = position;
}

void Camera::setDirection(Vector3 direction) {
	this->direction = direction.normalize();
}

void Camera::rotate(float x, float y) {
	rotate(x, y, 0.0);
}

void Camera::rotate(float x, float y, float z) {
	auto angle = x;
	float newX = direction.coords[1] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	float newY = direction.coords[1] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(direction.coords[0], newX, newY).normalize();

	angle = y;
	newX = direction.coords[0] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	newY = direction.coords[0] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(newX, direction.coords[1], newY).normalize();
}
