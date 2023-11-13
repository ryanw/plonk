#include "../helpers.h"
#include <plonk/math.h>

int math_matrices(int, char **) {
	it("Should init with an identity matrix", {
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

	return 0;
}
