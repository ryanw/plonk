#pragma once

#include <cstddef>
#include <cmath>
#include <iostream>

template <std::size_t Size, typename Derived>
class BaseVector {
public:
	float coords[Size];

	template <typename... Args>
	BaseVector(float first, Args... rest) {
		static_assert(sizeof...(Args) == Size - 1, "Invalid size");
		processConstructorArg(0, first, rest...);
	};

	BaseVector() {
		for (int i = 0; i < Size; i++) {
			coords[i] = 0.0;
		}
	}

	float x() const {
		return coords[0];
	}

	float y() const {
		return coords[1];
	}

	float z() const {
		return coords[2];
	}

	float w() const {
		return coords[3];
	}

	float magnitudeSquared() const {
		float mag = 0.0;
		for (int i = 0; i < Size; i++) {
			mag += std::abs(coords[i]);
		}
		return mag;
	}

	float magnitude() const {
		return std::sqrt(magnitudeSquared());
	}

	Derived normalize() const {
		auto mag = magnitude();

		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] / mag;
		}
		return vec;
	}

private:
	template <typename... Args>
	void processConstructorArg(int index, float first, Args... rest) {
		coords[index] = first;
		processConstructorArg(index + 1, rest...);
	}
	void processConstructorArg(int index) {}

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

	Vector<Size> operator-(const Point<Size> &other) const {
		Vector<Size> vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = this->coords[i] - other.coords[i];
		}
		return vec;
	}
};

typedef Vector<2> Vector2;
typedef Vector<3> Vector3;
typedef Vector<4> Vector4;
typedef Point<2> Point2;
typedef Point<3> Point3;
typedef Point<4> Point4;
