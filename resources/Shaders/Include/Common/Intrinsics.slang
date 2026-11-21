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

#ifndef PYRO_COMMON__INTRINSICS_SLANG
#define PYRO_COMMON__INTRINSICS_SLANG

#ifdef pyro_internal_enabled_glsl
	import glsl;
	#version 460;
#endif

#ifdef pyro_internal_shader_stage_vs
	#ifdef PYRO_SHADER_FLAG_ENABLE_GL_DRAW_ID
		//#define pyro_internal__DrawIndex gl_DrawID
	#else
		[[push_constant]] cbuffer pyro_internal__cbuffer_DrawIndex : register(b0, space2) { uint pyro_internal__DrawIndex; };
	#endif
#else
	#define pyro_internal__DrawIndex 0
#endif


#endif