#version 450
#define MAX_STEPS 1024
#define MAX_DIST 1024.0
#define SURFACE_DIST 0.01

layout(push_constant)
	uniform constants {
		float time;
	} PushConstants;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

struct DistanceResult {
	float d;
	vec3 color;
};

float sdfSphere(vec3 p, float rad) {
	return length(p) - rad;
}

float sdfBox(vec3 p, vec3 size) {
	vec3 q = abs(p) - size;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float opSmooth(float d0, float d1, float k) {
	return clamp(0.5 + 0.5 * (d1 - d0) / k, 0.0, 1.0);
}

float opSmoothUnion(float d0, float d1, float k) {
	float h = opSmooth(d0, d1, k);
	return mix(d1, d0, h) - k * h * (1.0 - h);
}

DistanceResult getDistance(vec3 p) {
	DistanceResult result;
	result.d = MAX_DIST + 1.0;
	float t = PushConstants.time;
	vec3 ballPos = vec3(0.0, 5.0 + sin(t * 2.0) * 9.0, 6.0);
	vec3 boxPos = vec3(0.0, 5.0, 12.0);
	float sphereDist = sdfSphere(p - ballPos, 3.0);
	float boxDist = sdfBox(p - boxPos, vec3(10.0, 0.3, 15.0));

	float d = opSmooth(sphereDist, boxDist, 7.0);
	result.d = opSmoothUnion(sphereDist, boxDist, 7.0);

	vec3 boxColor = vec3(0.3, 0.9, 0.1);
	vec3 ballColor = vec3(1.0, 0.1, 0.2);
	result.color = mix(boxColor, ballColor, d);

	return result;
}

DistanceResult rayMarch(vec3 ro, vec3 rd) {
	float d = 0.0;

	for (int i = 0; i < MAX_STEPS; i++) {
		vec3 p = ro + rd * d;
		DistanceResult surfaceDist = getDistance(p);
		d += surfaceDist.d;
		if (surfaceDist.d < SURFACE_DIST) {
			return DistanceResult(d, surfaceDist.color);
		}
		if (d > MAX_DIST) {
			// Sky colour
			return DistanceResult(d, vec3(0.1, 0.03, 0.2));
		}
	}

	return DistanceResult(d, vec3(0.0, 0.0, 1.0));
}


vec3 calcNormal(vec3 p) {
	float d = getDistance(p).d;

	float d0 = getDistance(p - vec3(SURFACE_DIST, 0.0, 0.0)).d;
	float d1 = getDistance(p - vec3(0.0, SURFACE_DIST, 0.0)).d;
	float d2 = getDistance(p - vec3(0.0, 0.0, SURFACE_DIST)).d;

	vec3 n = d - vec3(d0, d1, d2);
	return normalize(n);
}

float calcLight(vec3 p) {
	vec3 lightPos = vec3(10.0, -15.0, 2.0);
	//lightPos.xz += vec2(sin(PushConstants.time * 1.7), cos(PushConstants.time * 1.3)) * 13.0;
	vec3 lightDir = normalize(lightPos - p);
	vec3 n = calcNormal(p);

	float diffusion = clamp(dot(n, lightDir), 0.1, 1.0);

	float d = rayMarch(p + n * SURFACE_DIST * 2.0, lightDir).d;
	if (d < length(lightPos - p)) {
		diffusion = 0.1;
	}

	return diffusion;
}

void main() {
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	// Camera is just above the origin
	vec3 ro = vec3(0.0, -1.0, -12.0);
	// Screen is 1 in front of the camera -- controls FoV
	vec3 rd = normalize(vec3(uv.x, uv.y + 0.4, 1.0));

	DistanceResult dist = rayMarch(ro, rd);
	float d = dist.d;
	vec3 p = ro + rd * d;

	float diffusion = calcLight(p);
	color.rgb = dist.color * diffusion;
	outColor = color;
}
