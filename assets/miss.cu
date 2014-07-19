#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

#include "common.h"

using namespace optix;

rtDeclareVariable(Result, res, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(uint2 , index, rtLaunchIndex, );

rtBuffer<float4, 2> outBuffer;

rtTextureSampler<float4, 2> tex;
//rtTextureSampler<float4, cudaTextureTypeCubemap> tex2; //Kepler required...
rtDeclareVariable(float3, up, , );

RT_PROGRAM void miss(void) {
	float t = max(dot(ray.direction, up), 0.0f);
	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi   = M_PIf * 0.5f - acosf(ray.direction.y);
	float u     = (theta) * (0.5f * M_1_PIf);
	float v     = 0.5f * (1.0f + sin(phi));
	//texCubemap(tex2, u, u, u);
	res.color = make_float3(tex2D(tex, -u, -v));
}
