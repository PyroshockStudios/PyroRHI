#pragma once

static inline const char* gEXEC_TESS_SHADER = R"(
// Full tessellation chain: VS -> HS -> PatchConstants -> DS -> PS

struct VSOut {
    float3 position : POSITION;
    float3 color    : COLOR0;
};

VSOut vertexMain(uint vertexID : SV_VertexID)
{
    // Single triangular patch control points (you can change these or feed from a buffer)
    float3 positions[3] = {
        float3( 0.0f,  0.5f, 0.0f ),
        float3(-0.5f, -0.5f, 0.0f ),
        float3( 0.5f, -0.5f, 0.0f )
    };

    float3 colors[3] = {
        float3(1.0f, 0.2f, 0.2f),
        float3(0.2f, 1.0f, 0.2f),
        float3(0.2f, 0.2f, 1.0f)
    };

    VSOut outVS;
    outVS.position = positions[vertexID % 3]; // each patch uses 3 control points
    outVS.color = colors[vertexID % 3];
    return outVS;
}

struct HSOut {
    float3 position : POSITION;
    float3 color    : COLOR0;
};

// Patch constant structure that provides tess factors
struct HSPatchConst {
    float edges[3]  : SV_TessFactor;        // outer tess factors (edges)
    float inside    : SV_InsideTessFactor;  // inner tess factor
};

// Hull shader: output control points for the patch
[domain("tri")]
[partitioning("integer")]           // choose partitioning mode: integer / fractional_odd / fractional_even
[outputtopology("triangle_cw")]    // output primitive topology
[outputcontrolpoints(3)]           // number of control points we output
[patchconstantfunc("PatchConstants")]
HSOut hullMain(InputPatch<VSOut, 3> patch, uint cpID : SV_OutputControlPointID)
{
    HSOut outHS;
    outHS.position = patch[cpID].position; // pass-through control point
    outHS.color    = patch[cpID].color;
    return outHS;
}

// Patch constant function: set tessellation levels here (can be dynamic)
HSPatchConst PatchConstants(InputPatch<VSOut, 3> patch)
{
    HSPatchConst pc;
    // Example: fairly high subdivision to make tessellation visible
    pc.edges[0] = 8.0f;
    pc.edges[1] = 8.0f;
    pc.edges[2] = 8.0f;
    pc.inside   = 8.0f;
    return pc;
}

struct DSOut {
    float4 position : SV_Position;
    float3 color    : COLOR0;
};

[domain("tri")]
DSOut domainMain(HSPatchConst patchConsts, const OutputPatch<HSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    // bary = barycentric coordinates of the tessellated vertex
    // Interpolate control point positions / attributes using barycentric coords
    float3 pos = bary.x * patch[0].position + bary.y * patch[1].position + bary.z * patch[2].position;

    float3 col = bary.x * patch[0].color + bary.y * patch[1].color + bary.z * patch[2].color;

    DSOut outDS;
    outDS.position = float4(pos, 1.0f);
    outDS.color = col;
    return outDS;
}

float4 fragmentMain(DSOut input) : SV_Target
{
    return float4(input.color, 1.0f);
}

)";

static inline const char* gEXEC_GEOMETRY_SHADER = R"(
struct VSOutput {
    float4 position : SV_Position;
    float3 color    : COLOR0;
};

// Vertex shader - simple pass-through
VSOutput vertexMain(uint vertexID : SV_VertexID)
{
    float2 positions[3] = {
        float2( 0.0,  0.5),
        float2(-0.5, -0.5),
        float2( 0.5, -0.5)
    };
    float3 colors[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)
    };

    VSOutput o;
    o.position = float4(positions[vertexID], 0.0, 1.0);
    o.color = colors[vertexID];
    return o;
}

// Geometry shader: expands one triangle into 6 triangles in a fan
[maxvertexcount(18)] // 6 triangles and 3 vertices each
void geometryMain(triangle VSOutput input[3], inout TriangleStream<VSOutput> triStream)
{
    // Emit original triangle
    triStream.Append(input[0]);
    triStream.Append(input[1]);
    triStream.Append(input[2]);
    triStream.RestartStrip();

    // Expand around center
    float4 center = (input[0].position + input[1].position + input[2].position) / 3.0;

    for (uint i = 0; i < 3; ++i) {
		const float4 offset = float4(1.25, 1.25, 1.0, 1.0);
        VSOutput a = input[i];
		a.position *= offset;
        VSOutput b = input[(i + 1) % 3];
		b.position *= offset;

        VSOutput c; 
        c.position = (input[i].position + input[(i + 1) % 3].position) * offset * float4(1.5, 1.5, 1.0, 1.0);
        c.color = lerp(float3(1, 0, 0), float3(0,1,1), ((float)i) / 2.0);

        triStream.Append(a);
        triStream.Append(b);
        triStream.Append(c);
        triStream.RestartStrip();
    }
}

float4 fragmentMain(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
)";




static inline const char* gEXEC_RAY_QUERY_COMPUTE_SHADER = R"(
[[vk::binding(3, 0)]] RaytracingAccelerationStructure gTlas[] : register(t0, space1);
[[vk::binding(8, 1)]] RWByteAddressBuffer gOutput : register(u0, space1);

[[push_constant]] cbuffer pyro_PushConstant : register(b13, space0) { uint tlasIndex; };

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_NONE> query;

    query.TraceRayInline(
        gTlas[tlasIndex],                           // AccelerationStructure
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,   // uint RayFlags,
        0xFF,                                       // uint InstanceInclusionMask,
        ray,                                        // RayDesc
    );
    
    query.Proceed();

    uint hitResult = 0;
    if (query.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        hitResult = 1;
    }
    gOutput.Store(0, hitResult);
}
)";
