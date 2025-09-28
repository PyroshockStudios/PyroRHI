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