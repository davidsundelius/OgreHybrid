#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>
#include "common.h"

using namespace optix;

rtDeclareVariable(float3, normal, attribute shading_normal, ); 
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(Ray, ray, rtCurrentRay, );

rtDeclareVariable(float4, sphere, , );

RT_PROGRAM void sphereIntersect(int prim) {
	float3 center = make_float3(sphere.x, sphere.y, sphere.z);
	float radius = sphere.w;
	float3 O = ray.origin - center;
	float b = dot(O, ray.direction);
	float c = dot(O, O) - radius*radius;
	float disc = b*b - c;
	if(disc > 0.0f) {
		float sdisc = sqrtf(disc);
		float root1 = (-b - sdisc);
		bool check_second = true;
		if(rtPotentialIntersection(root1)) {
			normal = (root1*ray.direction + O) / radius;
			float3 cs = normalize(center-(root1*ray.direction + O));
			texcoord.x = 0.5-atan2(cs.z, cs.x)/(2*PI);
			texcoord.y = 0.5-2*asin(cs.y)/(2*PI);
			texcoord.z = 0.0f;
			if(rtReportIntersection(0)) {
				check_second = false;
			}
		}
		if(check_second) {
			float root2 = (-b + sdisc);
			if(rtPotentialIntersection(root2)) {
				normal = (root2*ray.direction + O) / radius;
				float3 cs = normalize(center-(root1*ray.direction + O));
				texcoord.x = 0.5-atan2(cs.z, cs.x)/(2*PI);
				texcoord.y = 0.5-2*asin(cs.y)/(2*PI);
				texcoord.z = 0.0f;
				rtReportIntersection(0);
			}
		}
	}
}

RT_PROGRAM void sphereBounds(int, float result[6]) {
	float3 cen = make_float3(sphere);
	float3 rad = make_float3(sphere.w);

	float3 min = cen - rad;
	float3 max = cen + rad;

	result[0] = min.x;
	result[1] = min.y;
	result[2] = min.z;
	result[3] = max.x;
	result[4] = max.y;
	result[5] = max.z;
}

