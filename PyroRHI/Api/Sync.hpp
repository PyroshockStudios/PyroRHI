#pragma once
#include <EASTL/span.h>
#include <EASTL/string.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <PyroRHI/Api/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {

        /**
         * @brief Parameters describing a buffer memory barrier.
         *
         * Used to synchronize access to buffer resources between pipeline stages or queues.
         */
        struct BufferMemoryBarrierInfo {
            /// @brief Buffer handle to apply the barrier to.
            Buffer buffer = PYRO_NULL_BUFFER;

            /// @brief Region of the buffer affected by this barrier.
            BufferRegion region = {};

            /// @brief Source access flags (before the barrier).
            Access srcAccess = AccessConsts::NONE;

            /// @brief Destination access flags (after the barrier).
            Access dstAccess = AccessConsts::NONE;

            /// @brief Buffer layout before the barrier. If `Undefined`, contents may not be preserved
            BufferLayout srcLayout = BufferLayout::Undefined;

            /// @brief Buffer layout after the barrier. *MUST* not be `Undefined`
            BufferLayout dstLayout = BufferLayout::Undefined;

            /// @brief Optional source queue for ownership transfer.
            /// @note if srcQueue is non-null, then dstQueue *MUST* be non-null, and be a different command queue
            ICommandQueue* srcQueue = nullptr;

            /// @brief Optional destination queue for ownership transfer.
            /// @note if dstQueue is non-null, then srcQueue *MUST* be non-null, and be a different command queue
            ICommandQueue* dstQueue = nullptr;

            PYRO_NODISCARD  bool operator==(const BufferMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const BufferMemoryBarrierInfo&) const = default;
        };

        /**
         * @brief Parameters describing an image memory barrier.
         *
         * Used to synchronize access to image resources between pipeline stages, layouts, or queues.
         */
        struct ImageMemoryBarrierInfo {
            /// @brief Image handle to apply the barrier to.
            Image image = PYRO_NULL_IMAGE;

            /// @brief Mip and array slice of the image affected by this barrier.
            ImageMipArraySlice imageSlice = {};

            /// @brief Source access flags (before the barrier).
            Access srcAccess = AccessConsts::NONE;

            /// @brief Destination access flags (after the barrier).
            Access dstAccess = AccessConsts::NONE;

            /// @brief Image layout before the barrier. If `Undefined`, contents may not be preserved
            ImageLayout srcLayout = ImageLayout::Undefined;

            /// @brief Image layout after the barrier. *MUST* not be `Undefined`
            ImageLayout dstLayout = ImageLayout::Undefined;

            /// @brief Optional source queue for ownership transfer.
            ICommandQueue* srcQueue = nullptr;

            /// @brief Optional destination queue for ownership transfer.
            ICommandQueue* dstQueue = nullptr;

            PYRO_NODISCARD  bool operator==(const ImageMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const ImageMemoryBarrierInfo&) const = default;
        };

        struct EventInfo {
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const EventInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const EventInfo&) const = default;
        };

        struct IEvent {
            IEvent() = default;

            PYRO_NODISCARD const EventInfo& Info() const;

        protected:
            virtual ~IEvent() = default;
            friend class IDevice;
        };

        struct EventSignalInfo {
            IEvent* event;
            PipelineStageFlags stage = {};
        };
        struct EventWaitInfo {
            IEvent* event;
            PipelineStageFlags srcStage = {};
            PipelineStageFlags dstStage = {};
            eastl::span<const BufferMemoryBarrierInfo> bufferBarriers = {};
            eastl::span<const ImageMemoryBarrierInfo> imageBarriers = {};
        };
    } // namespace RHI
} // namespace PyroshockStudios