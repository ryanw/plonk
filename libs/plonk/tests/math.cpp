#include <cassert>
#include <functional>
#include <iostream>
#include <plonk/math.h>

#define IT(name, ...) \
	if (!runTest(name, []() __VA_ARGS__)) \
		return false;

bool runTest(const char *testName, std::function<bool()> runner) {
	if (!runner()) {
		std::cout << "Failed: " << testName << "\n";
		return false;
	}

	return true;
}

bool vectors() {
	IT("Should have the correct magnitude", {
		Vector3 vec(0.0, 2.3, 0.0);
		return (std::abs(vec.magnitude() - 2.3) < 0.000001);
	});

	return true;
}

bool matrices() {
	IT("Should init with an identity matrix", {
		auto mat4 = Matrix4::identity();
		float expected[16] = {
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};

		for (int i = 0; i < 16; i++) {
			if (expected[i] != mat4[i])
				return false;
		}
		return true;
	});

	return true;
}

int math(int, char **) {
	if (!vectors())
		return 1;
	if (!matrices())
		return 1;
	return 0;
}
