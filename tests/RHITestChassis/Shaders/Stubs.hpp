#pragma once

static inline const char* gTEST_VERTEX_SHADER = R"(
float4 main(uint id : SV_VertexID) : SV_POSITION {
  const float4[] vertices = { float4(1., 0., 0., 1.), float4(0., 1., 0., 1.), float4(0., 0., 1., 1.)  };
  return vertices[id];
}
)";

static inline const char* gTEST_FRAGMENT_SHADER = R"(
float4 main() : SV_Target {
  return float4(1., 1., 1., 1.);
}
)";

static inline const char* gTEST_INPUT_ASSEMBLY_VERTEX_SHADER = R"(
struct VSInput {
    float3 position : POSITION0; // location 0
    float3 normal   : NORMAL0;   // location 1
};

float4 main(VSInput input) : SV_POSITION {
  return float4(input.position, 1.0);
}
)";



static inline const char* gTEST_RAY_QUERY_COMPUTE_SHADER = R"(
// Bindless TLAS array (as seen by Slang for cross-compatibility)
// We use the RHI's preferred binding indices
[[vk::binding(3, 0)]] RaytracingAccelerationStructure gTlas[] : register(t0, space1);

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Hardcode the TLAS index, not actually running anyway
    const uint tlasIndex = 1; 

    RayDesc ray;
    ray.Origin = float3(dispatchThreadID.x, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_NONE> query;

    query.TraceRayInline(
        gTlas[tlasIndex],                           // AccelerationStructure
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,   // uint RayFlags,
        ~0,                                         // uint InstanceInclusionMask,
        ray,                                        // RayDesc
    );

    uint hitResult = 0;
    if (query.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        hitResult = 1;
    }
}
)";
