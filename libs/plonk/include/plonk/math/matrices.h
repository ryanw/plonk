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

	Matrix4(
		float r0c0, float r0c1, float r0c2, float r0c3,
		float r1c0, float r1c1, float r1c2, float r1c3,
		float r2c0, float r2c1, float r2c2, float r2c3,
		float r3c0, float r3c1, float r3c2, float r3c3
	) {
		data[0] = r0c0;
		data[1] = r0c1;
		data[2] = r0c2;
		data[3] = r0c3;

		data[4] = r1c0;
		data[5] = r1c1;
		data[6] = r1c2;
		data[7] = r1c3;

		data[8] = r2c0;
		data[9] = r2c1;
		data[10] = r2c2;
		data[11] = r2c3;

		data[12] = r3c0;
		data[13] = r3c1;
		data[14] = r3c2;
		data[15] = r3c3;
	};

	int operator[](int index) {
		return data[index];
	}

private:
	float data[16];
};
