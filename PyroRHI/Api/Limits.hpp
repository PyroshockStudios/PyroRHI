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
#include <PyroCommon/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        namespace Limits {
            /// @brief Max push constant offset + size in bytes allowed to be passed per stage.
            constexpr u32 MAX_PUSH_CONSTANT_SIZE = 128;
            /// @brief Max uniform buffer offset + size in bytes bound
            constexpr u32 MAX_UNIFORM_BUFFER_SIZE = 65535;
            /// @brief Max unorderd access views that can be bound at once
            constexpr u32 MAX_UNORDERED_ACCESS_VIEW_SLOTS = 12;
            /// @brief Max uniform buffer views that can be bound at once
            constexpr u32 MAX_UNIFORM_BUFFER_VIEW_SLOTS = 8;
            /// @brief Max color render targets that can be set at once in a render pass
            constexpr u32 MAX_BOUND_COLOR_TARGETS = 8;
            /// @brief Max specialisation constants can be set per shader stage
            constexpr u32 MAX_SPECIALIZATION_CONSTANTS = 16U;
        } // namespace Limits
    } // namespace RHI
} // namespace PyroshockStudios