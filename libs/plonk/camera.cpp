#include "include/plonk/camera.h"

Camera::Camera() {}

Camera::~Camera() {}

void Camera::setPosition(Point3 position) {
	this->position = position;
}

void Camera::setDirection(Vector3 direction) {
	this->direction = direction.normalize();
}
