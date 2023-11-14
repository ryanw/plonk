#include "../helpers.h"
#include <plonk/math.h>

describe(math_matrices, {
	it("creates an identity matrix", {
		auto mat4 = Matrix4::identity();
		float expected[16] = {
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};

		for (int i = 0; i < 16; i++) {
			assert(expected[i] == mat4[i], "Invalid identity matrix");
		}
	});

	it("transforms a Point3", {
		Matrix4 mat(
			1.0, 0.0, 4.0, 0.0,
			0.0, 1.0, 0.0, 2.0,
			2.0, 0.0, 3.0, 0.0,
			0.0, 7.0, 1.0, 2.0
		);
		Point3 p(4.0, 5.0, 6.0);
		Point3 result = mat * p;

		assert_approx(result.x(), 1.333);
		assert_approx(result.y(), 1.000);
		assert_approx(result.z(), 2.917);
	});

	it("transforms a Vector3", {
		Matrix4 mat(
			1.0, 0.0, 4.0, 0.0,
			0.0, 1.0, 0.0, 2.0,
			2.0, 0.0, 3.0, 0.0,
			0.0, 7.0, 1.0, 2.0
		);
		Vector3 vec(4.0, 5.0, 6.0);
		Vector3 result = mat * vec;

		assert_approx(result.x(), 16.0);
		assert_approx(result.y(), 5.0);
		assert_approx(result.z(), 34.0);
	});

	it("transforms a Vector4", {
		Matrix4 mat(
			1.0, 0.0, 4.0, 0.0,
			0.0, 1.0, 0.0, 2.0,
			2.0, 0.0, 3.0, 0.0,
			0.0, 7.0, 1.0, 2.0
		);
		Vector4 vec(4.0, 5.0, 6.0, 1.0);
		Vector4 result = mat * vec;

		assert_approx(result.x(), 16.0);
		assert_approx(result.y(), 12.0);
		assert_approx(result.z(), 35.0);
		assert_approx(result.w(), 12.0);
	});

	it("multiplies matrices together", {
		Matrix4 mat0(
			1, 0, 4, 0,
			0, 1, 0, 2,
			2, 0, 3, 0,
			0, 7, 1, 2
		);

		Matrix4 mat1(
			3, 3, 5, 8,
			7, 1, 4, 5,
			2, 3, 2, 1,
			3, 5, 0, 1
		);

		Matrix4 expected(
			13, 59, 35, 22,
			15, 36, 45, 12,
			6, 10, 15, 8,
			3, 12, 13, 12
		);

		Matrix4 result = mat0 * mat1;
		for (int i = 0; i < 16; i++) {
			assert_approx(result[i], expected[i]);
		}
	});

	it("rotates a Point3", {
		Point3 point(4.0, 5.0, 6.0);
		Matrix4 mat = Matrix4::from_rotation(0.0, M_PI / 4.0, 0.0);
		Point3 result = mat * point;
		Point3 expected(7.071, 5.0, 1.414);
		for (int i = 0; i < 3; i++) {
			assert_approx(result[i], expected[i]);
		}
	});
});
