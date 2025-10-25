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

#include <PyroRHI/Core.hpp>
#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        /**
         * @brief Parameters used to create a fence.
         */
        struct FenceInfo {
            /// @brief Initial fence value. Default is 0.
            u64 initialValue = {};

            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const FenceInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const FenceInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief GPU fence interface.
         *
         * Used to synchronize GPU and CPU operations.
         */
        struct IFence {
            IFence() = default;

            /**
             * @brief Get the description for this fence.
             */
            PYRO_NODISCARD virtual const FenceInfo& Info() const = 0;

            /**
             * @brief Get the current fence value.
             *
             * @return u64 Current value of the fence.
             */
            PYRO_NODISCARD virtual u64 Value() const = 0;

            /**
             * @brief Update the fence value.
             *
             * @param value New value to set.
             */
            virtual void SetValue(u64 value) = 0;

            /**
             * @brief Wait for the fence to reach at least the given value.
             *
             * @param value Value to wait for.
             * @param timeoutNs Maximum wait time in nanoseconds (default infinite).
             * @return true if the fence reached the value within the timeout.
             * @note A timeout of infinity is not recommended, as GPU hang can be indefinite.
             */
            PYRO_NODISCARD virtual bool WaitForValue(u64 value, u64 timeoutNs = ~u64(0)) = 0;

        protected:
            virtual ~IFence() = default;
            friend class IDevice;
        };
    } // namespace RHI
} // namespace PyroshockStudios