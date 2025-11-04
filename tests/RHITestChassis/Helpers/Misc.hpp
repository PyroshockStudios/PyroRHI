#pragma once
#include <PyroCommon/Types.hpp>
using namespace PyroshockStudios::Types;
// helper function
template <typename T>
T HIGHEST_BIT(T x) {
    for (u64 k = 1ULL << (sizeof(x) * 8 - 1); k != 0; k >>= 1) {
        if ((static_cast<u64>(x) & k) == k)
            return static_cast<T>(k);
    }
    return static_cast<T>(0);
}