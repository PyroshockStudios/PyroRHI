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

#ifndef PYRO_COMMON__SPECIALIZATION_CONSTANT_SLANG
#define PYRO_COMMON__SPECIALIZATION_CONSTANT_SLANG

#ifdef PYRO_SHADER_FLAG_ENABLE_SPECIALIZATION_CONSTANTS
#define PYRO_SPECIALIZATION(slot, type, name, defaultValue) [[vk::constant_id(slot)]] const type name = (type)(defaultValue);
#else

#ifdef pyro_internal_shader_stage_cs
#define pyro_internal_spec_const_Register_For_Stage b8
#elif defined(pyro_internal_shader_stage_vs)
#define pyro_internal_spec_const_Register_For_Stage b8
#elif defined(pyro_internal_shader_stage_hs)
#define pyro_internal_spec_const_Register_For_Stage b9
#elif defined(pyro_internal_shader_stage_ds)
#define pyro_internal_spec_const_Register_For_Stage b10
#elif defined(pyro_internal_shader_stage_gs)
#define pyro_internal_spec_const_Register_For_Stage b11
#elif defined(pyro_internal_shader_stage_fs)
#define pyro_internal_spec_const_Register_For_Stage b12
#endif

cbuffer pyro_internal__PyroSpecConst : register(pyro_internal_spec_const_Register_For_Stage, space0) 
{
    uint4 pyro_internal__Specialization[16]; 
}

#define PYRO_SPECIALIZATION(slot, type, name, defaultValue) \
	static const type name = pyro_internal__Specialization[slot].y == 1 ? reinterpret<type>(pyro_internal__Specialization[slot].x) : (type)(defaultValue)

#endif

#endif 
