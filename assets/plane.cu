#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(float3, normal, attribute shading_normal, ); 
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(Ray, ray, rtCurrentRay, );

rtDeclareVariable(float3, n, , );
rtDeclareVariable(float3, pos, , );


RT_PROGRAM void planeIntersect(int prim) {
	float den = dot(n, ray.direction);
	if(den < 0.0f) {
		float3 hitPoint = pos-ray.origin;
		float d = dot(hitPoint, n) / den;
		//if(hitPoint >= 0) {
			if(rtPotentialIntersection(d)) {
				normal = n;
				texcoord.x = 0.0f;
				texcoord.y = 0.0f;
				texcoord.z = 0.0f;
				rtReportIntersection(0);
			}
		//}
	}
}

RT_PROGRAM void planeBounds(int, float result[6]) {
	float3 min = n+pos - 10;//- 1e-6;
	float3 max = n+pos + 10;//+ 1e-6;

	result[0] = min.x;
	result[1] = min.y;
	result[2] = min.z;
	result[3] = max.x;
	result[4] = max.y;
	result[5] = max.z;
}

