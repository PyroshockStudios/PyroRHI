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
#include <EASTL/string.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        /**
         * @brief Parameters used to create a semaphore.
         */
        struct SemaphoreInfo {
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const SemaphoreInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SemaphoreInfo&) const = default;
        };

        /**
         * @brief GPU semaphore handle.
         *
         * Used for synchronization between command queues or submissions.
         */
        RHI_TYPED_PTR_HANDLE(Semaphore);
    } // namespace RHI
} // namespace PyroshockStudios