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

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Types.hpp>
#include <PyroRHI/Api/GPUResource.hpp>

namespace PyroshockStudios {
    inline namespace RHI {

        /**
         * @brief Supported formats for a swap chain's images.
         *
         * Specifies the color format and dynamic range for the back buffers.
         */
        enum struct SwapChainFormat : u32 {
            /** Standard 8-bit per channel, low dynamic range (sRGB not applied). */
            Unorm8BitLDR = 0,

            /** Standard 8-bit per channel, low dynamic range with sRGB encoding. */
            Srgb8BitLDR = 1,

            /** 10-bit per channel, low dynamic range. */
            Unorm10BitLDR = 2,

            /** 16-bit floating point per channel, high dynamic range. */
            Float16BitHDR = 3
        };

        /**
         * @brief Specifies how the alpha channel of the swap chain's back buffers is interpreted.
         *
         * Determines how the window compositor (DWM on windows) blends the swap chain with the desktop
         * or other windows when the swap chain has transparency.
         */
        enum struct SwapChainAlphaMode : u32 {
            /** Ignore the alpha channel; the swap chain is treated as fully opaque. */
            None = 0,

            /** Premultiplied alpha; the color channels are already multiplied by the alpha value. */
            Premultiplied = 1,

            /** Postmultiplied alpha; color channels are not pre-multiplied and will be multiplied during composition. */
            Postmultiplied = 2
        };

        enum struct SwapChainPresentMode : i32 {
            Tearing = 0,
            LowLatency = 1,
            VSync = 2,
            VSyncAdaptive = 3
        };


        /**
         * @brief Parameters for creating a swap chain.
         *
         * Describes the target window, image format, presentation mode, and number of back buffers.
         */
        struct SwapChainInfo {
            /**
             * @brief Native platform window handle.
             * Used by the swap chain to present images to the OS window.
             */
            NativeHandle nativeWindow = {};

            /**
             * @brief Native platform instance handle.
             * Platform-specific context or instance. 
             * In Win32 this is the HINSTANCE, while in linux we use the X11 implementation, so a Display is required.
             * MacOS Cocoa does not have such an instance, and can be left NULL. 
             */
            NativeHandle nativeInstance = {};

            /**
             * @brief Image format for the swap chain's back buffers.
             * Default is `Unorm8BitLDR`.
             */
            SwapChainFormat format = SwapChainFormat::Unorm8BitLDR;

            /**
             * @brief What alpha mode the swap chain has, used for composition on backgrounds.
             */
            SwapChainAlphaMode alphaMode = SwapChainAlphaMode::None;

            /**
             * @brief Presentation mode, controlling vsync and tearing behavior.
             */
            SwapChainPresentMode presentMode = SwapChainPresentMode::VSync;

            /**
             * @brief Number of back buffers in the swap chain.
             * Typically 2 or 3 for double or triple buffering.
             * @note This value may be clamped to the swapchain's requirements. Never assume the input bufferCount is the created bufferCount,
             * make sure to query this true value by calling ISwapChain::Info()
             * NOTE: this MUST be larger or equal to number of frames in flight!
             */
            u32 bufferCount = 2;

            /**
             * @brief Intended usage flags for swap chain images.
             * At least 1 image usage is REQUIRED to be set
             */
            ImageUsageFlags imageUsage = ImageUsageFlagBits::NONE;

            /**
             * @brief Initial dimensions of the swap chain images.
             */
            Extent2D extent = {};

            /**
             * @brief Optional human-readable name for debugging and profiling.
             */
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const SwapChainInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const SwapChainInfo&) const = default;
        };

        static constexpr inline i32 PYRO_SWAPCHAIN_ACQUIRE_FAIL = -1;

        /**
         * @brief Interface representing a GPU swap chain.
         *
         * Provides access to back buffers, handles image acquisition and presentation,
         * and allows resizing and format configuration.
         */
        struct ISwapChain {
            ISwapChain() = default;
            virtual ~ISwapChain() = default;

            /**
             * @brief Get a back buffer by index.
             *
             * @param imageIndex Index of the back buffer.
             * @return Image Handle to the back buffer.
             */
            PYRO_NODISCARD virtual Image GetBackBuffer(i32 imageIndex) = 0;

            /**
             * @brief Acquire the next available image for rendering.
             *
             * @return Image index to the next back buffer.
             * Returns PYRO_SWAPCHAIN_ACQUIRE_FAIL if acquisition failed (e.g., swap chain needs resize).
             */
            PYRO_NODISCARD virtual i32 AcquireNextImage() = 0;

            /** @brief Resize the swap chain to match the current window size. */
            virtual void Resize() = 0;

            /**
             * @brief Change the swap chain's present mode.
             *
             * @param presentMode New present mode (e.g., VSync, Immediate).
             */
            virtual void SetPresentMode(SwapChainPresentMode presentMode) = 0;

            /**
             * @brief Get the description of this swap chain.
             *
             * @return const SwapChainInfo& Reference to swap chain creation info.
             */
            PYRO_NODISCARD virtual const SwapChainInfo& Info() const = 0;

            /**
             * @brief Get the current size of the swap chain images.
             *
             * @return Extent2D Current image width and height.
             */
            PYRO_NODISCARD virtual Extent2D GetSurfaceExtent() const = 0;

            /**
             * @brief Get the swap chain image format.
             *
             * @return Format Current pixel format.
             */
            PYRO_NODISCARD virtual Format GetFormat() const = 0;

            /**
             * @brief Get the swap chain's color space.
             *
             * @return ColorSpace Current color space of the back buffers.
             */
            PYRO_NODISCARD virtual ColorSpace GetColorSpace() const = 0;
        };

    } // namespace RHI
} // namespace PyroshockStudios
