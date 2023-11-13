#include "vectors.h"
#include <array>
#include <cstring>

class Matrix4 {
public:
	static Matrix4 identity() {
		return Matrix4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}

	Matrix4() {}
	Matrix4(Matrix4 &other) {
		std::memcpy(data, other.data, sizeof(other.data));
	}
	Matrix4(Matrix4 *other) {
		std::memcpy(data, other->data, sizeof(other->data));
	}

	Matrix4(
		float c0r0, float c0r1, float c0r2, float c0r3,
		float c1r0, float c1r1, float c1r2, float c1r3,
		float c2r0, float c2r1, float c2r2, float c2r3,
		float c3r0, float c3r1, float c3r2, float c3r3
	) {
		data[0] = c0r0;
		data[1] = c0r1;
		data[2] = c0r2;
		data[3] = c0r3;

		data[4] = c1r0;
		data[5] = c1r1;
		data[6] = c1r2;
		data[7] = c1r3;

		data[8] = c2r0;
		data[9] = c2r1;
		data[10] = c2r2;
		data[11] = c2r3;

		data[12] = c3r0;
		data[13] = c3r1;
		data[14] = c3r2;
		data[15] = c3r3;
	};

	std::array<Vector4, 4> columns() {
		return {
			Vector4(data[0], data[1], data[2], data[3]),
			Vector4(data[4], data[5], data[6], data[7]),
			Vector4(data[8], data[9], data[10], data[11]),
			Vector4(data[12], data[13], data[14], data[15]),
		};
	}


	std::array<Vector4, 4> rows() {
		return {
			Vector4(data[0], data[4], data[8], data[12]),
			Vector4(data[1], data[5], data[9], data[13]),
			Vector4(data[2], data[6], data[10], data[14]),
			Vector4(data[3], data[7], data[11], data[15]),
		};
	}

	Matrix4 operator*(Matrix4 &other) {
		Matrix4 result(this);
		auto other_cols = other.columns();
		std::array<Vector4, 4> scaled_cols = {
			*this * other_cols[0],
			*this * other_cols[1],
			*this * other_cols[2],
			*this * other_cols[3],
		};

		for (int i = 0; i < 4; i++) {
			result.data[0 + i * 4] = scaled_cols[i][0];
			result.data[1 + i * 4] = scaled_cols[i][1];
			result.data[2 + i * 4] = scaled_cols[i][2];
			result.data[3 + i * 4] = scaled_cols[i][3];
		}

		return result;
	}

	Vector4 operator*(Vector4 &other) {
		Vector4 scaled[4];
		auto cols = columns();
		for (int i = 0; i < cols.size(); i++) {
			scaled[i] = cols[i] * other[i];
		}

		auto x = scaled[0];
		auto y = scaled[1];
		auto z = scaled[2];
		auto w = scaled[3];
		return Vector4(
			x[0] + y[0] + z[0] + w[0],
			x[1] + y[1] + z[1] + w[1],
			x[2] + y[2] + z[2] + w[2],
			x[3] + y[3] + z[3] + w[3]
		);
	}

	float operator[](int index) {
		return data[index];
	}

private:
	float data[16];
};
