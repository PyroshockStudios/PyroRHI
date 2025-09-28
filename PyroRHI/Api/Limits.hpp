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