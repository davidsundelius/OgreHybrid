#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

#include "common.h"

using namespace optix;

#define epsilon 0.001f

rtDeclareVariable(uint2 , index    , rtLaunchIndex, );

rtDeclareVariable(uint, rayTypeRadience, , );
rtDeclareVariable(float3, camPos   , , );
rtDeclareVariable(float3, u, , );
rtDeclareVariable(float3, v, , );
rtDeclareVariable(float3, w, , );

rtDeclareVariable(uint , numSamples, , );

rtDeclareVariable(rtObject, scene, , );
rtDeclareVariable(int, skipMask, , );
rtTextureSampler<float, 2> mask;
rtBuffer<float4, 2> outBuffer;


RT_PROGRAM void generate() {
	float2 sc = make_float2(index) / make_float2(outBuffer.size());
	if(tex2D(mask, sc.x, sc.y) != 0.0f || skipMask) {
		float2 d = sc * 2.0f - 1.0f;
		float3 orgin = camPos;
		float3 direction = w + d.x*u - d.y*v;
		Result res;
		float3 color = make_float3(0.0f);
		float3 tmpDir;
		for(int i=0;i<numSamples;i++) {
			if(i==0) tmpDir = direction;
			else if(i==1) tmpDir = direction+epsilon*u+epsilon*v;
			else if(i==2) tmpDir = direction-epsilon*u+epsilon*v;
			else if(i==3) tmpDir = direction-epsilon*u-epsilon*v;
			else tmpDir = direction+epsilon*u-epsilon*v;
			Ray r = make_Ray(orgin, tmpDir, rayTypeRadience, 0.005f, RT_DEFAULT_MAX);
			res.depth = 0;
			res.importance = 1.0f;
			rtTrace(scene, r, res);
			color+=res.color;
		}
		color/=numSamples;
		outBuffer[index] = make_float4(color,1.0f);
	} else {
		outBuffer[index] = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}


RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	if( code == RT_EXCEPTION_STACK_OVERFLOW )
		outBuffer[index] = make_float4(1.0f,0.0f,1.0f,1.0f);
	else
		rtPrintExceptionDetails();
}