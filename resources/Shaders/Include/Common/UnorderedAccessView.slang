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

#ifndef PYRO_COMMON__UNORDERED_ACCESS_VIEW_SLANG
#define PYRO_COMMON__UNORDERED_ACCESS_VIEW_SLANG

#define PYRO_BIND_UNORDERED_ACCESS_STRUCTURED_BUFFER(slot, type, name)      [[vk::binding(slot + 8, 1)]] RWStructuredBuffer<type> name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_BYTE_ADDRESS_BUFFER(slot, name)    		[[vk::binding(slot + 8, 1)]] RWByteAddressBuffer name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_TEXTURE_1D(slot, type, name)             [[vk::binding(slot + 20, 1)]] RWTexture1D<type> name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_TEXTURE_2D(slot, type, name)             [[vk::binding(slot + 20, 1)]] RWTexture2D<type> name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_TEXTURE_3D(slot, type, name)             [[vk::binding(slot + 20, 1)]] RWTexture3D<type> name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_TEXTURE_1D_ARRAY(slot, type, name)       [[vk::binding(slot + 20, 1)]] RWTexture1DArray<type> name : register(u##slot, space1)
#define PYRO_BIND_UNORDERED_ACCESS_TEXTURE_2D_ARRAY(slot, type, name)       [[vk::binding(slot + 20, 1)]] RWTexture2DArray<type> name : register(u##slot, space1)

#endif 
