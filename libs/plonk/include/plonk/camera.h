#pragma once

#include "math.h"

class Camera
{
public:
	Point3 position = Point3(0.0, 0.0, -3.0);
	Vector3 direction = {0.0, 0.0, 1.0};

	Camera();
	~Camera();

	void set_position(Point3 position);
	void set_direction(Vector3 direction);
	void rotate(float x, float y);
	void translate(float x, float y, float z);

protected:
};
