#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

#include "common.h"

using namespace optix;

rtDeclareVariable(float3, normal, attribute shading_normal, );
rtDeclareVariable(float3, gnormal, attribute geometric_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, hit, rtIntersectionDistance, );
rtDeclareVariable(Result, res, rtPayload, );
rtDeclareVariable(ResultShadow, sRes, rtPayload, );

rtDeclareVariable(rtObject, scene, , );
rtDeclareVariable(uint, rayTypeShadow, , );
rtDeclareVariable(uint, rayTypeRadience, , );
rtDeclareVariable(float3, ambientColor, , );
rtDeclareVariable(float3, ambient, , );
rtDeclareVariable(float3, diffuse, , );
rtDeclareVariable(float3, specular, , );
rtDeclareVariable(float3, emissive, , );
rtDeclareVariable(float, shininess, , );
rtDeclareVariable(float, reflexive, , );
rtDeclareVariable(float, opacity, , );
rtDeclareVariable(float, refractiveIndex, , );
rtDeclareVariable(int, isTextured, , );
rtTextureSampler<float4, 2> sampler;
rtBuffer<Light> lights;


RT_PROGRAM void closestHit() {
	float3 n = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, normal));
	float3 gn = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, gnormal));
	float3 ffn = faceforward(n, -ray.direction, gn);
	float3 hitPoint = ray.origin + hit * ray.direction;

	//Ambient
	float3 tA;
	if(isTextured) {
		tA = opacity*make_float3(tex2D(sampler, texcoord.x, texcoord.y))*ambient*0.3f;
	} else {
		tA = (1.0-reflexive)*opacity*ambient*ambientColor;
	}

	//Reflections
	float3 tRefl = make_float3(0.0f);
	if(reflexive>0.0f && res.depth<MAXDEPTH && res.importance>MINIMPORTANCE) {
		float s = fresnel_schlick(-dot(ffn, ray.direction), 5, reflexive, 0.8f);
		Result reflRes;
		reflRes.importance = res.importance*optix::luminance(make_float3(reflexive));
		reflRes.depth = res.depth+1;
		float3 rv = reflect(ray.direction, ffn);
		Ray reflRay = make_Ray(hitPoint, rv, rayTypeRadience, 0.05f, RT_DEFAULT_MAX);
		rtTrace(scene, reflRay, reflRes);
		tRefl = s * reflRes.color;
	}

	//Direct
	float3 tD = make_float3(0.0f);
	if(opacity>0.0f || res.depth>=MAXDEPTH) {
		for(int i=0; i<lights.size(); ++i) {
			Light l = lights[i];
			float3 lv = normalize(l.pos - hitPoint);
			float theta = dot(ffn, lv);
			if(theta > 0.0f) {
				//Shadows
				ResultShadow sRes;
				sRes.attenuation = 1.0f;
				float ld = length(l.pos - hitPoint);
				Ray sRay = make_Ray(hitPoint, l.pos, rayTypeShadow, 0.1f, ld);
				rtTrace(scene, sRay, sRes);
				if(sRes.attenuation > 0.0f) {
					//Diffuse
					float3 lc = l.color*sRes.attenuation;
					float3 diff;
					if(isTextured) {
						diff = make_float3(tex2D(sampler, texcoord.x, texcoord.y))*theta*lc;
					} else {
						diff = diffuse*theta*lc;
					}
					float3 spec = make_float3(0.0f);
					float3 lrv = reflect(lv, ffn);
					float phi = dot(lrv, normalize(ray.direction));
					if(phi > 0) {
						spec = specular*powf(phi,shininess)*l.color;
					}
					tD += opacity*(diff+spec)*(1.0-reflexive);
				}
			}
		}
		tD/=lights.size();
	}

	//Refractions
	float3 tRefr = make_float3(0.0f);
	if(opacity<1.0f && res.depth<MAXDEPTH && res.importance>MINIMPORTANCE) {
		float3 rv;
		if(refract(rv, ray.direction, ffn, refractiveIndex)) {
			float t = dot(ray.direction, ffn);
			if(t < 0.0f)
				t = -t;
			else
				t = dot(rv, n);
			Result refrRes;
			refrRes.importance = res.importance*optix::luminance(make_float3(1.0f-opacity));
			refrRes.depth = res.depth+1;
			Ray refrRay = make_Ray(hitPoint, rv, rayTypeRadience, 0.05f, RT_DEFAULT_MAX);
			rtTrace(scene, refrRay, refrRes);
			tRefr += (1.0f-opacity)*refrRes.color;
		}
	}
	res.color = tA + tD + tRefr + tRefl + emissive;
}

RT_PROGRAM void anyHitShadow() {
	if(opacity==1.0f) {
		sRes.attenuation=0.0f;
		rtTerminateRay();
	} else {
		float3 n = normalize(normal);
		float theta = fabs(dot(n, ray.direction));
		//Caustics (very approximated)
		float ri = clamp(refractiveIndex-1.0f, 0.0f, 1.0f);
		if(ri>0.01f) {
			float exp = powf(2.0f,lerp(0.0f,9.0f,ri));
			sRes.attenuation *= powf(theta, exp)*refractiveIndex*lerp(1.0f,5.0f,ri);
		}
		//Shadow
		sRes.attenuation *= 1.0f-fresnel_schlick(theta, 5, opacity, 1);

		float luminance = optix::luminance(make_float3(sRes.attenuation));
		if(luminance < 0.01f || luminance > 1.0f) {
			rtTerminateRay();
		} else {
			rtIgnoreIntersection();
		}
	}
}
