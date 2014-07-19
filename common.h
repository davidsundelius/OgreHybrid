#pragma once

#include <Optix/optix_math.h>

#define MAXDEPTH 5
#define MINIMPORTANCE 0.3
#define PI 3.14159265

struct Result {
	int depth;
	float importance;
	float3 color;
};

struct ResultShadow {
	float attenuation;
};

struct Light {
	float3 pos;
	float3 color;
	int castShadows;
	int padding;
};