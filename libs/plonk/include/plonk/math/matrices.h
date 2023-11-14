#include "vectors.h"
#include <array>
#include <cstring>
#include <optional>

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
	Matrix4(const Matrix4 &other) {
		std::memcpy(data, other.data, sizeof(other.data));
	}
	Matrix4(const Matrix4 *other) {
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

	static Matrix4 from_translation(float x, float y, float z) {
		Matrix4 mat(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			x, y, z, 1.0
		);

		return mat;
	}

	static Matrix4 from_rotation(float x, float y, float z) {
		float cx = std::cos(x);
		float sx = std::sin(x);
		float cy = std::cos(y);
		float sy = std::sin(y);
		float cz = std::cos(z);
		float sz = std::sin(z);

		Matrix4 rotx(
			1, 0, 0, 0,
			0, cx, sx, 0,
			0, -sx, cx, 0,
			0, 0, 0, 1
		);

		Matrix4 roty(
			cy, 0, -sy, 0,
			0, 1, 0, 0,
			sy, 0, cy, 0,
			0, 0, 0, 1
		);

		Matrix4 rotz(
			cz, sz, 0, 0,
			-sz, cz, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);

		return rotz * roty * rotx;
	}

	static Matrix4 from_scaling(float x, float y, float z) {
		Matrix4 mat(
			x, 0.0, 0.0, 0.0,
			0.0, y, 0.0, 0.0,
			0.0, 0.0, z, 0.0,
			0.0, 0.0, 0.0, 1.0
		);

		return mat;
	}

	std::array<Vector4, 4> columns() const {
		return {
			Vector4(data[0], data[1], data[2], data[3]),
			Vector4(data[4], data[5], data[6], data[7]),
			Vector4(data[8], data[9], data[10], data[11]),
			Vector4(data[12], data[13], data[14], data[15]),
		};
	}


	std::array<Vector4, 4> rows() const {
		return {
			Vector4(data[0], data[4], data[8], data[12]),
			Vector4(data[1], data[5], data[9], data[13]),
			Vector4(data[2], data[6], data[10], data[14]),
			Vector4(data[3], data[7], data[11], data[15]),
		};
	}

	float determinant() const {
		float m00 = data[0],
			m01 = data[1],
			m02 = data[2],
			m03 = data[3],
			m10 = data[4],
			m11 = data[5],
			m12 = data[6],
			m13 = data[7],
			m20 = data[8],
			m21 = data[9],
			m22 = data[10],
			m23 = data[11],
			m30 = data[12],
			m31 = data[13],
			m32 = data[14],
			m33 = data[15];

		float det =
			(m00 * m11 * m22 * m33) -
			(m00 * m11 * m23 * m32) +
			(m00 * m12 * m23 * m31) -
			(m00 * m12 * m21 * m33) +

			(m00 * m13 * m21 * m32) -
			(m00 * m13 * m22 * m31) -
			(m01 * m12 * m23 * m30) +
			(m01 * m12 * m20 * m33) -

			(m01 * m13 * m20 * m32) +
			(m01 * m13 * m22 * m30) -
			(m01 * m10 * m22 * m33) +
			(m01 * m10 * m23 * m32) +

			(m02 * m13 * m20 * m31) -
			(m02 * m13 * m21 * m30) +
			(m02 * m10 * m21 * m33) -
			(m02 * m10 * m23 * m31) +

			(m02 * m11 * m23 * m30) -
			(m02 * m11 * m20 * m33) -
			(m03 * m10 * m21 * m32) +
			(m03 * m10 * m22 * m31) -

			(m03 * m11 * m22 * m30) +
			(m03 * m11 * m20 * m32) -
			(m03 * m12 * m20 * m31) +
			(m03 * m12 * m21 * m30);

		return det;
	}

	std::optional<Matrix4> inverse() const {
		Matrix4 mat = Matrix4::identity();
		float det = determinant();
		if (det == 0.0) return std::nullopt;

		float d = 1.0 / det;

		float b0 = (data[2] * data[7]) - (data[6] * data[3]);
		float b1 = (data[2] * data[11]) - (data[10] * data[3]);
		float b2 = (data[14] * data[3]) - (data[2] * data[15]);
		float b3 = (data[6] * data[11]) - (data[10] * data[7]);
		float b4 = (data[14] * data[7]) - (data[6] * data[15]);
		float b5 = (data[10] * data[15]) - (data[14] * data[11]);

		float a0 = (data[0] * data[5]) - (data[4] * data[1]);
		float a1 = (data[0] * data[9]) - (data[8] * data[1]);
		float a2 = (data[12] * data[1]) - (data[0] * data[13]);
		float a3 = (data[4] * data[9]) - (data[8] * data[5]);
		float a4 = (data[12] * data[5]) - (data[4] * data[13]);
		float a5 = (data[8] * data[13]) - (data[12] * data[9]);

		float d11 = (data[5] * b5) + (data[9] * b4) + (data[13] * b3);
		float d12 = (data[1] * b5) + (data[9] * b2) + (data[13] * b1);
		float d13 = (data[1] * -b4) + (data[5] * b2) + (data[13] * b0);
		float d14 = (data[1] * b3) + (data[5] * -b1) + (data[9] * b0);

		float d21 = (data[4] * b5) + (data[8] * b4) + (data[12] * b3);
		float d22 = (data[0] * b5) + (data[8] * b2) + (data[12] * b1);
		float d23 = (data[0] * -b4) + (data[4] * b2) + (data[12] * b0);
		float d24 = (data[0] * b3) + (data[4] * -b1) + (data[8] * b0);

		float d31 = (data[7] * a5) + (data[11] * a4) + (data[15] * a3);
		float d32 = (data[3] * a5) + (data[11] * a2) + (data[15] * a1);
		float d33 = (data[3] * -a4) + (data[7] * a2) + (data[15] * a0);
		float d34 = (data[3] * a3) + (data[7] * -a1) + (data[11] * a0);

		float d41 = (data[6] * a5) + (data[10] * a4) + (data[14] * a3);
		float d42 = (data[2] * a5) + (data[10] * a2) + (data[14] * a1);
		float d43 = (data[2] * -a4) + (data[6] * a2) + (data[14] * a0);
		float d44 = (data[2] * a3) + (data[6] * -a1) + (data[10] * a0);

		mat[0] = d11 * d;
		mat[4] = -(d21 * d);
		mat[8] = d31 * d;
		mat[12] = -(d41 * d);

		mat[1] = -(d12 * d);
		mat[5] = d22 * d;
		mat[9] = -(d32 * d);
		mat[13] = d42 * d;

		mat[2] = d13 * d;
		mat[6] = -(d23 * d);
		mat[10] = d33 * d;
		mat[14] = -(d43 * d);

		mat[3] = -(d14 * d);
		mat[7] = d24 * d;
		mat[11] = -(d34 * d);
		mat[15] = d44 * d;


		return std::optional(mat);
	}

	Matrix4 operator*(Matrix4 &other) const {
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

	Vector4 operator*(Vector4 other) const {
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

	Vector3 operator*(Vector3 other) const {
		Vector4 vec = *this * Vector4(other[0], other[1], other[2], 0.0);
		return Vector3(
			vec.x(),
			vec.y(),
			vec.z()
		);
	}

	Point3 operator*(Point3 other) const {
		Vector4 vec = *this * Vector4(other[0], other[1], other[2], 1.0);
		float w = vec.w();
		return Point3(
			vec.x() / w,
			vec.y() / w,
			vec.z() / w
		);
	}

	float &operator[](int index) {
		return data[index];
	}

	const float &operator[](int index) const {
		return data[index];
	}

private:
	float data[16];
};
