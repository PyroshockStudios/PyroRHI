// MIT License
//
// Copyright (c) 2025 Pyroshock Studios
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

#pragma once
#include <PyroCommon/Platform.hpp>

//-------------------------------------------------------------------------------------------------
// Handle Declarations
//-------------------------------------------------------------------------------------------------

/**
 * @brief Defines a strongly-typed pointer handle for a GPU resource type.
 */
#define RHI_TYPED_PTR_HANDLE(x) typedef struct x##_T* x;

/**
 * @brief Defines a strongly-typed 64-bit handle for a GPU resource type.
 *
 * If compiling on a 64-bit platform, the handle is simply a pointer type.
 * On non 64-bit platforms, the handle is represented as a struct with an internal 64-bit value.
 */
#ifdef PYRO_PLATFORM_64_BIT
#define RHI_TYPED_HANDLE64(x) RHI_TYPED_PTR_HANDLE(x);
#else
#define RHI_TYPED_HANDLE64(x)                                                                    \
    typedef struct x##_T {                                                                       \
        u64 m_unused = 0;                              /**< Raw handle value. */                 \
        operator u64() { return m_unused; }            /**< Implicit conversion to raw value. */ \
        operator bool() { return m_unused > 0; }       /**< Returns true if handle is valid. */  \
        bool operator==(const x##_T&) const = default; /**< Equality comparison operator. */     \
        bool operator!=(const x##_T&) const = default; /**< Inequality comparison operator. */   \
    } x
#endif