#pragma once

#include <cstddef>
#include <cmath>
#include <iostream>

template <std::size_t Size>
class Vector;

template <std::size_t Size, typename Derived>
class BaseVector {
public:
	float coords[Size];

	template <typename... Args>
	BaseVector(float first, Args... rest) {
		static_assert(sizeof...(Args) == Size - 1, "Invalid size");
		process_constructor_arg(0, first, rest...);
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

	template <typename T>
	float dot(const BaseVector<Size, T> &other) const {
		float result = 0.0;
		for (int i = 0; i < Size; i++) {
			result += coords[i] * other.coords[i];
		}
		return result;
	}

	template <typename T>
	Vector<Size> cross(const BaseVector<Size, T> &other) const {
		static_assert(Size == 3, "not a 3D vector");
		auto x = this->y() * other.z() - this->z() * other.y();
		auto y = this->z() * other.x() - this->x() * other.z();
		auto z = this->x() * other.y() - this->y() * other.x();
		Vector<Size> result(x, y, z);
		return result;
	}

	float magnitude_squared() const {
		float mag = 0.0;
		for (int i = 0; i < Size; i++) {
			mag += std::pow(std::abs(coords[i]), 2);
		}
		return mag;
	}

	float magnitude() const {
		return std::sqrt(magnitude_squared());
	}

	Derived normalize() const {
		auto mag = magnitude();

		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] / mag;
		}
		return vec;
	}

	template <typename T>
	Derived operator+(const BaseVector<Size, T> &other) const {
		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] + other.coords[i];
		}
		return vec;
	}

	template <typename T>
	Derived operator-(const BaseVector<Size, T> &other) const {
		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] - other.coords[i];
		}
		return vec;
	}

	Derived operator*(float amount) const {
		Derived vec;
		for (int i = 0; i < Size; i++) {
			vec.coords[i] = coords[i] * amount;
		}
		return vec;
	}

private:
	template <typename... Args>
	void process_constructor_arg(int index, float first, Args... rest) {
		coords[index] = first;
		process_constructor_arg(index + 1, rest...);
	}
	void process_constructor_arg(int index) {}

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
