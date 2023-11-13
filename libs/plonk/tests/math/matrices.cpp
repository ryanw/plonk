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

	it("transforms a vector", {
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
});
