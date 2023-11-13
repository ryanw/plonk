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
	auto angle = y;
	auto newX = direction.coords[0] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	auto newY = direction.coords[0] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(newX, direction.coords[1], newY).normalize();

	angle = x;
	newX = direction.coords[1] * std::cos(angle) - direction.coords[2] * std::sin(angle);
	newY = direction.coords[1] * std::sin(angle) + direction.coords[2] * std::cos(angle);
	direction = Vector3(direction.coords[0], newX, newY).normalize();

}

void Camera::translate(float x, float y, float z) {
	position = position + direction * z;
	//position.coords[0] += x * direction.coords[0];
	//position.coords[1] += y * direction.coords[1];
	//position.coords[2] += z * direction.coords[2];
}
