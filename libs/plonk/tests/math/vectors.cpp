#include "../helpers.h"
#include <plonk/math.h>

describe(math_vectors, {
	it("should have the correct magnitude", {
		Vector3 vec(0.0, 2.3, 0.0);
		assert_approx(vec.magnitude(), 2.3);
	});

	it("calculates the dot product", {
		Vector3 v0(4.0, 7.0, 2.0);
		Vector3 v1(8.0, 3.0, 8.0);
		float dot = v0.dot(v1);
		assert_approx(dot, 69.0);
	});

	it("calculates the cross product", {
		Vector3 v0(3.0, 9.0, 14.0);
		Vector3 v1(7.0, 3.0, 19.0);
		Vector3 cross = v0.cross(v1);
		Vector3 expected(129.0, 41.0, -54.0);
		assert_approx(cross.x(), expected.x());
		assert_approx(cross.y(), expected.y());
		assert_approx(cross.z(), expected.z());
	});
});
