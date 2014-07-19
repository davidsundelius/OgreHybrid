#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>
#include <Optix/optixu/optixu_aabb_namespace.h>

using namespace optix;

rtDeclareVariable(float3, normal, attribute normal, ); 
rtDeclareVariable(Ray, ray, rtCurrentRay, );

rtBuffer<float3> vertexBuffer;     
rtBuffer<float3> normalBuffer;
//rtBuffer<float2> texcoordBuffer;
rtBuffer<int3>   vIndexBuffer;
rtBuffer<int3>   nIndexBuffer;
//rtBuffer<int3>   tIndexBuffer;

//rtBuffer<uint>   materialBuffer;
rtDeclareVariable(float2, texcoord, attribute texcoord, ); 

RT_PROGRAM void meshIntersect(int prim) {
  int3 vid = vIndexBuffer[prim];
  float3 p0 = vertexBuffer[vid.x];
  float3 p1 = vertexBuffer[vid.y];
  float3 p2 = vertexBuffer[vid.z];

  // Intersect ray with triangle
  float3 n;
  float  t, beta, gamma;
  if(intersect_triangle(ray, p0, p1, p2, n, t, beta, gamma)) {
    if(rtPotentialIntersection(t)) {
      //int3 nid = nIndexBuffer[prim];
      //if (normalBuffer.size() == 0 || nid.x < 0 || nid.y < 0 || nid.z < 0) {
        normal = normalize(n);
      /*} else {
        float3 n0 = normalBuffer[nid.x];
        float3 n1 = normalBuffer[nid.y];
        float3 n2 = normalBuffer[nid.z];
        normal = normalize(n1*beta + n2*gamma + n0*(1.0f-beta-gamma));
      }*/

	  texcoord = make_float2(0.0f, 0.0f);

      /*int3 tid = tIndexBuffer[prim];
      if (texcoordBuffer.size() == 0 || tid.x < 0 || tid.y < 0 || tid.z < 0) {
        
      } else {
        float2 t0 = texcoordBuffer[tid.x];
        float2 t1 = texcoordBuffer[tid.y];
        float2 t2 = texcoordBuffer[tid.z];
        texcoord = make_float3(t1*beta + t2*gamma + t0*(1.0f-beta-gamma));
      }*/
      rtReportIntersection(0); //material_buffer[primIdx]
    }
  }
}

RT_PROGRAM void meshBounds (int prim, float result[6]) {  
  const int3 vid = vIndexBuffer[prim];
  const float3 v0   = vertexBuffer[vid.x];
  const float3 v1   = vertexBuffer[vid.y];
  const float3 v2   = vertexBuffer[vid.z];
  const float area = length(cross(v1-v0, v2-v0));

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if(area > 0.0f && !isinf(area)) {
    aabb->m_min = fminf(fminf(v0, v1), v2);
    aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
  } else {
    aabb->invalidate();
  }
}
