// MIT License
//
// Copyright (c) 2025-2026 Pyroshock Studios
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef COMMON__DESCRIPTOR_INDEXING_SLANG
#define COMMON__DESCRIPTOR_INDEXING_SLANG

typedef uint32_t PyroDescriptor;

#define pyro_internal_access(index, resource) resource[index] // TODO add debug bounds check
#define PYRO_ACCESS(index, resource) pyro_internal_access(index, resource)

// Byte address buffers
[[vk::binding(0)]] ByteAddressBuffer        gByteBuffer[]        : register(t0, space1);

// Textures (SRVs)
[[vk::binding(1)]] Texture1D<float4>        gTexture1D[]         : register(t0, space2);
[[vk::binding(1)]] Texture2D<float4>        gTexture2D[]         : register(t0, space3);
[[vk::binding(1)]] TextureCube<float4>      gTextureCube[]       : register(t0, space4);
[[vk::binding(1)]] Texture3D<float4>        gTexture3D[]         : register(t0, space5);
[[vk::binding(1)]] Texture1DArray<float4>   gTexture1DArray[]    : register(t0, space6);
[[vk::binding(1)]] Texture2DArray<float4>   gTexture2DArray[]    : register(t0, space7);
[[vk::binding(1)]] TextureCubeArray<float4> gTextureCubeArray[]  : register(t0, space8);
[[vk::binding(1)]] Texture2DMS<float4>      gTexture2DMS[]       : register(t0, space9);
[[vk::binding(1)]] Texture2DMSArray<float4> gTexture2DMSArray[]  : register(t0, space10);

// Unsigned Textures (SRVs)
[[vk::binding(1)]] Texture1D<uint4>        gTexture1Du[]         : register(t0, space11);
[[vk::binding(1)]] Texture2D<uint4>        gTexture2Du[]         : register(t0, space12);
[[vk::binding(1)]] TextureCube<uint4>      gTextureCubeu[]       : register(t0, space13);
[[vk::binding(1)]] Texture3D<uint4>        gTexture3Du[]         : register(t0, space14);
[[vk::binding(1)]] Texture1DArray<uint4>   gTexture1DArrayu[]    : register(t0, space15);
[[vk::binding(1)]] Texture2DArray<uint4>   gTexture2DArrayu[]    : register(t0, space16);
[[vk::binding(1)]] TextureCubeArray<uint4> gTextureCubeArrayu[]  : register(t0, space17);
[[vk::binding(1)]] Texture2DMS<uint4>      gTexture2DMSu[]       : register(t0, space18);
[[vk::binding(1)]] Texture2DMSArray<uint4> gTexture2DMSArrayu[]  : register(t0, space19);

// Signed Textures (SRVs)
[[vk::binding(1)]] Texture1D<int4>        gTexture1Di[]          : register(t0, space20);
[[vk::binding(1)]] Texture2D<int4>        gTexture2Di[]          : register(t0, space21);
[[vk::binding(1)]] TextureCube<int4>      gTextureCubei[]        : register(t0, space22);
[[vk::binding(1)]] Texture3D<int4>        gTexture3Di[]          : register(t0, space23);
[[vk::binding(1)]] Texture1DArray<int4>   gTexture1DArrayi[]     : register(t0, space24);
[[vk::binding(1)]] Texture2DArray<int4>   gTexture2DArrayi[]     : register(t0, space25);
[[vk::binding(1)]] TextureCubeArray<int4> gTextureCubeArrayi[]   : register(t0, space26);
[[vk::binding(1)]] Texture2DMS<int4>      gTexture2DMSi[]        : register(t0, space27);
[[vk::binding(1)]] Texture2DMSArray<int4> gTexture2DMSArrayi[]   : register(t0, space28);

// Samplers
[[vk::binding(2)]] SamplerState             gSampler[]           : register(s0, space30);
[[vk::binding(2)]] SamplerComparisonState   gSamplerComparison[] : register(s0, space31);


#endif 
