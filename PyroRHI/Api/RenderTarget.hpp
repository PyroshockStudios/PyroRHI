#pragma once
#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <PyroRHI/Api/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {

        struct IDevice;
        
        struct RenderTargetFlagsProperties {
            using Data = u32;
        };

        using RenderTargetFlags = Flags<RenderTargetFlagsProperties>;
        struct RenderTargetFlagBits {
            static inline constexpr RenderTargetFlags NONE = { 0x00000000 };
            static inline constexpr RenderTargetFlags COLOR_TARGET = { 0x00000001 };
            static inline constexpr RenderTargetFlags DEPTH_TARGET = { 0x00000002 };
            static inline constexpr RenderTargetFlags STENCIL_TARGET = { 0x00000004 };
            static inline constexpr RenderTargetFlags DEPTH_STENCIL_TARGET = DEPTH_TARGET | STENCIL_TARGET;
        };

        /**
         * @brief Parameters for creating or referencing a render target.
         *
         * This struct contains the GPU image and subresource slice used for a render target,
         * along with an optional human-readable name for debugging or profiling purposes.
         */
        struct RenderTargetInfo {
            /**
             * @brief Handle to the GPU image to be used as a render target.
             * *MUST* be a valid Image handle.
             */
            Image image = PYRO_NULL_IMAGE;

            /**
             * @brief Slice of the image (mip level, array layer) to use for rendering.
             */
            ImageSlice slice = {};
            
            /**
             * @brief Aspect flags of the render target. 
             * If the format is NOT a depth-stencil format, `COLOR_TARGET` must be set. Otherwise, `DEPTH_TARGET` or `STENCIL_TARGET` or both must be set.
             */
            RenderTargetFlags flags = RenderTargetFlagBits::NONE;

            /**
             * @brief Optional debug name
             */
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const RenderTargetInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const RenderTargetInfo&) const = default;
        };

        /// @brief Handle to a GPU render target. Can be either a colour target, or a depth-stencil target
        RHI_TYPED_PTR_HANDLE(RenderTarget);

    } // namespace RHI
} // namespace PyroshockStudios
