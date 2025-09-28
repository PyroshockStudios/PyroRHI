#pragma once
#include <PyroRHI/Core.hpp>
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