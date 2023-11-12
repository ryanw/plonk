#pragma once

#include "math.h"

class Camera
{
public:
	Point3 position = Point3(0.0, 0.0, -3.0);
	Vector3 direction = {0.0, 0.0, 1.0};

	Camera();
	~Camera();

	void setPosition(Point3 position);
	void setDirection(Vector3 direction);

protected:
};
