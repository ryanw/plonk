#pragma once

#include <cstddef>
#include <cmath>

template <std::size_t Size, typename Derived>
class BaseVector {
public:
	float coords[Size];

	template <typename... Args>
	BaseVector(Args... args) : coords(args...) {
		static_assert(sizeof...(Args) == Size, "Invalid size");
	};

	BaseVector() {
		for (int i = 0; i < Size; i++) {
			coords[i] = 0.0;
		}
	}

	float magnitudeSquared() {
		float mag = 0.0;
		for (int i = 0; i < Size; i++) {
			mag += coords[i];
		}
		return mag;
	}

	float magnitude() {
		return std::sqrt(magnitudeSquared());
	}

	Derived normalize() {
		auto mag = magnitude();

		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] / mag;
		}
		return vec;
	}

};

template <std::size_t Size>
class Vector : public BaseVector<Size, Vector<Size>> {
public:
	template <typename... Args>
	Vector(Args... args) : BaseVector<Size, Vector<Size>>(args...) {}
};

template <std::size_t Size>
class Point : public BaseVector<Size, Point<Size>> {
public:
	template <typename... Args>
	Point(Args... args) : BaseVector<Size, Point<Size>>(args...) {}
};

typedef Vector<2> Vector2;
typedef Vector<3> Vector3;
typedef Vector<4> Vector4;
typedef Point<2> Point2;
typedef Point<3> Point3;
typedef Point<4> Point4;
