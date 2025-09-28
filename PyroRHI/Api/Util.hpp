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
#include "Types.hpp"

namespace PyroshockStudios {
    namespace RHIUtil {
        /**
         * @brief Numeric type classification of a GPU format.
         *
         * Useful for deciding how to interpret or convert a format.
         */
        enum struct FormatNumericType {
            Float,       ///< Floating-point formats (including normalized and sRGB).
            SignedInt,   ///< Signed integer formats.
            UnsignedInt, ///< Unsigned integer formats.
            Mixed,       ///< Formats with multiple numeric types (e.g., depth-stencil).
            Other,       ///< Any format not covered by the above.
        };
        /**
         * @brief Get the numeric type of a GPU format.
         *
         * @param type Format enum value.
         * @return FormatNumericType Classification of the format.
         */
        static constexpr FormatNumericType GetFormatNumericType(Format type) {
            switch (type) {
            // Floats (explicit float + Unorm, Snorm, Srgb)
            case Format::R16Sfloat:
            case Format::RG16Sfloat:
            case Format::RGB16Sfloat:
            case Format::RGBA16Sfloat:
            case Format::R32Sfloat:
            case Format::RG32Sfloat:
            case Format::RGB32Sfloat:
            case Format::RGBA32Sfloat:
                // case Format::R64Sfloat:
                // case Format::RG64Sfloat:
                // case Format::RGB64Sfloat:
                // case Format::RGBA64Sfloat:
            case Format::D32Sfloat:
            case Format::BC6HUfloatBlock:
            case Format::BC6HSfloatBlock:
            case Format::B10GR11Ufloat:
            case Format::E5BGR9Ufloat:

            // Unorm formats

            // case Format::RG4Unorm:
            // case Format::RGBA4Unorm:
            case Format::BGRA4Unorm:
                // case Format::RGB565Unorm:
            case Format::BGR565Unorm:
                // case Format::RGB5A1Unorm:
            case Format::BGR5A1Unorm:
                //  case Format::A1RGB5Unorm:
            case Format::R8Unorm:
            case Format::RG8Unorm:
            case Format::RGB8Unorm:
            case Format::RGBA8Unorm:
            case Format::BGR8Unorm:
            case Format::BGRA8Unorm:
                // case Format::ABGR8Unorm:
            case Format::A2RGB10Unorm:
                // case Format::A2BGR10Unorm:
            case Format::R16Unorm:
            case Format::RG16Unorm:
            case Format::RGB16Unorm:
            case Format::RGBA16Unorm:
            case Format::D16Unorm:
            case Format::X8D24Unorm:
            case Format::BC1RGBUnormBlock:
            case Format::BC1RGBAUnormBlock:
            case Format::BC2UnormBlock:
            case Format::BC3UnormBlock:
            case Format::BC4UnormBlock:
            case Format::BC5UnormBlock:
                // case Format::ARGB4Unorm:
                // case Format::ABGR4Unorm:

            // Snorm formats
            case Format::R8Snorm:
            case Format::RG8Snorm:
            case Format::RGB8Snorm:
            case Format::RGBA8Snorm:
            case Format::BGR8Snorm:
            case Format::BGRA8Snorm:
                // case Format::ABGR8Snorm:
            case Format::A2RGB10Snorm:
                // case Format::A2BGR10Snorm:
            case Format::R16Snorm:
            case Format::RG16Snorm:
            case Format::RGB16Snorm:
            case Format::RGBA16Snorm:
            case Format::BC4SnormBlock:
            case Format::BC5SnormBlock:

            // Srgb formats
            case Format::R8Srgb:
            case Format::RG8Srgb:
            case Format::RGB8Srgb:
            case Format::RGBA8Srgb:
            case Format::BGR8Srgb:
            case Format::BGRA8Srgb:
                // case Format::ABGR8Srgb:
            case Format::BC1RGBSrgbBlock:
            case Format::BC1RGBASrgbBlock:
            case Format::BC2SrgbBlock:
            case Format::BC3SrgbBlock:
            case Format::BC7SrgbBlock:
                return FormatNumericType::Float;

            // Signed integer formats
            case Format::R8Sint:
            case Format::RG8Sint:
            case Format::RGB8Sint:
            case Format::RGBA8Sint:
            case Format::BGR8Sint:
            case Format::BGRA8Sint:
                // case Format::ABGR8Sint:
            case Format::A2RGB10Sint:
                // case Format::A2BGR10Sint:
            case Format::R16Sint:
            case Format::RG16Sint:
            case Format::RGB16Sint:
            case Format::RGBA16Sint:
            case Format::R32Sint:
            case Format::RG32Sint:
            case Format::RGB32Sint:
            case Format::RGBA32Sint:
                // case Format::R64Sint:
                // case Format::RG64Sint:
                // case Format::RGB64Sint:
                // case Format::RGBA64Sint:
                return FormatNumericType::SignedInt;

            // Unsigned integer formats
            case Format::R8Uint:
            case Format::RG8Uint:
            case Format::RGB8Uint:
            case Format::RGBA8Uint:
            case Format::BGR8Uint:
            case Format::BGRA8Uint:
                // case Format::ABGR8Uint:
            case Format::A2RGB10Uint:
                // case Format::A2BGR10Uint:
            case Format::R16Uint:
            case Format::RG16Uint:
            case Format::RGB16Uint:
            case Format::RGBA16Uint:
            case Format::R32Uint:
            case Format::RG32Uint:
            case Format::RGB32Uint:
            case Format::RGBA32Uint:
                // case Format::R64Uint:
                // case Format::RG64Uint:
                // case Format::RGB64Uint:
                // case Format::RGBA64Uint:
            case Format::S8Uint:
                return FormatNumericType::UnsignedInt;

            case Format::D32SfloatS8Uint:
            case Format::D16UnormS8Uint:
            case Format::D24UnormS8Uint:
                return FormatNumericType::Mixed;

            // Everything else
            default:
                return FormatNumericType::Other;
            }
        }
        /**
         * @brief Returns the number of components in a format.
         *
         * Examples:
         * - R8Unorm -> 1
         * - RGBA8Unorm -> 4
         * - BC1 block format -> 4
         */
        static i32 GetFormatComponentCount(Format format) {
            switch (format) {
            // 1 component (R only)
            case Format::R8Unorm:
            case Format::R8Snorm:
            case Format::R8Uint:
            case Format::R8Sint:
            case Format::R8Srgb:
            case Format::R16Unorm:
            case Format::R16Snorm:
            case Format::R16Uint:
            case Format::R16Sint:
            case Format::R16Sfloat:
            case Format::R32Uint:
            case Format::R32Sint:
            case Format::R32Sfloat:
                // case Format::R64Uint:
                // case Format::R64Sint:
                // case Format::R64Sfloat:
            case Format::S8Uint:
            case Format::D16Unorm:
            case Format::D32Sfloat:
                // ignore stencil for the depth buffer as it works differently
            case Format::D16UnormS8Uint:
            case Format::D24UnormS8Uint:
            case Format::X8D24Unorm:
            case Format::D32SfloatS8Uint:
                return 1;

                // 2 components (RG)
                // case Format::RG4Unorm:
            case Format::RG8Unorm:
            case Format::RG8Snorm:
            case Format::RG8Uint:
            case Format::RG8Sint:
            case Format::RG8Srgb:
            case Format::RG16Unorm:
            case Format::RG16Snorm:
            case Format::RG16Uint:
            case Format::RG16Sint:
            case Format::RG16Sfloat:
            case Format::RG32Uint:
            case Format::RG32Sint:
            case Format::RG32Sfloat:
                // case Format::RG64Uint:
                // case Format::RG64Sint:
                // case Format::RG64Sfloat:
            case Format::BC5SnormBlock:
            case Format::BC5UnormBlock:
                return 2;

                // 3 components (RGB, BGR)
                // case Format::RGB565Unorm:
            case Format::BGR565Unorm:
            case Format::RGB8Unorm:
            case Format::RGB8Snorm:
            case Format::RGB8Uint:
            case Format::RGB8Sint:
            case Format::RGB8Srgb:
            case Format::BGR8Unorm:
            case Format::BGR8Snorm:
            case Format::BGR8Uint:
            case Format::BGR8Sint:
            case Format::BGR8Srgb:
            case Format::RGB16Unorm:
            case Format::RGB16Snorm:
            case Format::RGB16Uint:
            case Format::RGB16Sint:
            case Format::RGB16Sfloat:
            case Format::RGB32Uint:
            case Format::RGB32Sint:
            case Format::RGB32Sfloat:
                // case Format::RGB64Uint:
                // case Format::RGB64Sint:
                // case Format::RGB64Sfloat:
            case Format::BC1RGBUnormBlock:
            case Format::BC1RGBSrgbBlock:
            case Format::BC6HUfloatBlock:
            case Format::BC6HSfloatBlock:
            case Format::B10GR11Ufloat:
            case Format::E5BGR9Ufloat:
                return 3;

                // 4 components (RGBA, BGRA, ABGR, ARGB, A2RGB10, etc.)
                // case Format::RGBA4Unorm:
            case Format::BGRA4Unorm:
                // case Format::RGB5A1Unorm:
            case Format::BGR5A1Unorm:
                // case Format::A1RGB5Unorm:
            case Format::RGBA8Unorm:
            case Format::RGBA8Snorm:
            case Format::RGBA8Uint:
            case Format::RGBA8Sint:
            case Format::RGBA8Srgb:
            case Format::BGRA8Unorm:
            case Format::BGRA8Snorm:
            case Format::BGRA8Uint:
            case Format::BGRA8Sint:
            case Format::BGRA8Srgb:
                // case Format::ABGR8Unorm:
                // case Format::ABGR8Snorm:
                // case Format::ABGR8Uint:
                // case Format::ABGR8Sint:
                // case Format::ABGR8Srgb:
            case Format::A2RGB10Unorm:
            case Format::A2RGB10Snorm:
            case Format::A2RGB10Uint:
            case Format::A2RGB10Sint:
                // case Format::A2BGR10Unorm:
                // case Format::A2BGR10Snorm:
                // case Format::A2BGR10Uint:
                // case Format::A2BGR10Sint:
            case Format::RGBA16Unorm:
            case Format::RGBA16Snorm:
            case Format::RGBA16Uint:
            case Format::RGBA16Sint:
            case Format::RGBA16Sfloat:
            case Format::RGBA32Uint:
            case Format::RGBA32Sint:
            case Format::RGBA32Sfloat:
                // case Format::RGBA64Uint:
                // case Format::RGBA64Sint:
                // case Format::RGBA64Sfloat:
            case Format::BC1RGBAUnormBlock:
            case Format::BC1RGBASrgbBlock:
            case Format::BC2UnormBlock:
            case Format::BC2SrgbBlock:
            case Format::BC3UnormBlock:
            case Format::BC3SrgbBlock:
            case Format::BC7UnormBlock:
            case Format::BC7SrgbBlock:
                // case Format::ARGB4Unorm:
                // case Format::ABGR4Unorm:
                return 4;

            default:
                return 0;
            }
        }

        /**
         * @brief Returns the size in bytes of a single element of the given format.
         *
         * For block-compressed formats, returns bytes per block.
         * Returns 0 if the format has inconsistent size (e.g., D32SfloatS8Uint) or is unknown.
         */
        static constexpr u32 GetFormatSize(Format format) {
            switch (format) {
            // 8-bit per channel formats
            case Format::R8Unorm:
            case Format::R8Snorm:
            case Format::R8Uint:
            case Format::R8Sint:
            case Format::R8Srgb:
                return 1;
            case Format::RG8Unorm:
            case Format::RG8Snorm:
            case Format::RG8Uint:
            case Format::RG8Sint:
            case Format::RG8Srgb:
                return 2;
            case Format::RGB8Unorm:
            case Format::RGB8Snorm:
            case Format::RGB8Uint:
            case Format::RGB8Sint:
            case Format::RGB8Srgb:
            case Format::BGR8Unorm:
            case Format::BGR8Snorm:
            case Format::BGR8Uint:
            case Format::BGR8Sint:
            case Format::BGR8Srgb:
                return 3;
            case Format::RGBA8Unorm:
            case Format::RGBA8Snorm:
            case Format::RGBA8Uint:
            case Format::RGBA8Sint:
            case Format::RGBA8Srgb:
            case Format::BGRA8Unorm:
            case Format::BGRA8Snorm:
            case Format::BGRA8Uint:
            case Format::BGRA8Sint:
            case Format::BGRA8Srgb:
                return 4;

            // Packed 10:10:10:2
            case Format::A2RGB10Unorm:
            case Format::A2RGB10Snorm:
            case Format::A2RGB10Uint:
            case Format::A2RGB10Sint:
                return 4;

            // 16-bit per channel formats
            case Format::R16Unorm:
            case Format::R16Snorm:
            case Format::R16Uint:
            case Format::R16Sint:
            case Format::R16Sfloat:
                return 2;
            case Format::RG16Unorm:
            case Format::RG16Snorm:
            case Format::RG16Uint:
            case Format::RG16Sint:
            case Format::RG16Sfloat:
                return 4;
            case Format::RGB16Unorm:
            case Format::RGB16Snorm:
            case Format::RGB16Uint:
            case Format::RGB16Sint:
            case Format::RGB16Sfloat:
                return 6;
            case Format::RGBA16Unorm:
            case Format::RGBA16Snorm:
            case Format::RGBA16Uint:
            case Format::RGBA16Sint:
            case Format::RGBA16Sfloat:
                return 8;

            // 32-bit per channel formats
            case Format::R32Uint:
            case Format::R32Sint:
            case Format::R32Sfloat:
                return 4;
            case Format::RG32Uint:
            case Format::RG32Sint:
            case Format::RG32Sfloat:
                return 8;
            case Format::RGB32Uint:
            case Format::RGB32Sint:
            case Format::RGB32Sfloat:
                return 12;
            case Format::RGBA32Uint:
            case Format::RGBA32Sint:
            case Format::RGBA32Sfloat:
                return 16;

            // Special packed formats
            case Format::B10GR11Ufloat:
                return 4;
            case Format::E5BGR9Ufloat:
                return 4;

            // Depth/stencil formats
            case Format::D16Unorm:
                return 2;
            case Format::X8D24Unorm:
                return 4;
            case Format::D32Sfloat:
                return 4;
            case Format::S8Uint:
                return 1;
            case Format::D16UnormS8Uint:
                return 3; // 2 + 1
            case Format::D24UnormS8Uint:
                return 4; // 3 + 1
            case Format::D32SfloatS8Uint:
                return 0; // inconsistent

            // BC compressed formats (bytes per 4x4 block)
            case Format::BC1RGBUnormBlock:
            case Format::BC1RGBSrgbBlock:
            case Format::BC1RGBAUnormBlock:
            case Format::BC1RGBASrgbBlock:
                return 8; // 4x4 block = 8 bytes
            case Format::BC2UnormBlock:
            case Format::BC2SrgbBlock:
            case Format::BC3UnormBlock:
            case Format::BC3SrgbBlock:
            case Format::BC5UnormBlock:
            case Format::BC5SnormBlock:
            case Format::BC7UnormBlock:
            case Format::BC7SrgbBlock:
                return 16; // 4x4 block = 16 bytes
            case Format::BC4UnormBlock:
            case Format::BC4SnormBlock:
                return 8; // 4x4 block = 8 bytes
            case Format::BC6HUfloatBlock:
            case Format::BC6HSfloatBlock:
                return 16; // 4x4 block = 16 bytes
            }
            return 0; // Unknown format
        }

        /**
         * @brief Computes the required staging buffer size for an image with the given dimensions, format and row alignment.
         *
         * Accounts for block-compressed formats.
         */
        static constexpr DeviceSize GetRequiredStagingSize(Format format, u32 width, u32 height, u32 depth, u32 rowAlignment) {
            constexpr u32 blockWidth = 4;
            constexpr u32 blockHeight = 4;

            switch (format) {
            // 8-byte block formats
            case Format::BC1RGBUnormBlock:
            case Format::BC1RGBSrgbBlock:
            case Format::BC1RGBAUnormBlock:
            case Format::BC1RGBASrgbBlock:
            case Format::BC4UnormBlock:
            case Format::BC4SnormBlock: {
                const u32 blocksX = (width + blockWidth - 1) / blockWidth;
                const u32 blocksY = (height + blockHeight - 1) / blockHeight;
                const DeviceSize rowBytes = PYRO_ALIGN(blocksX * 8, rowAlignment); // 8 bytes per block
                return rowBytes * blocksY * depth;
            }

            // 16-byte block formats
            case Format::BC2UnormBlock:
            case Format::BC2SrgbBlock:
            case Format::BC3UnormBlock:
            case Format::BC3SrgbBlock:
            case Format::BC5UnormBlock:
            case Format::BC5SnormBlock:
            case Format::BC6HUfloatBlock:
            case Format::BC6HSfloatBlock:
            case Format::BC7UnormBlock:
            case Format::BC7SrgbBlock: {
                const u32 blocksX = (width + blockWidth - 1) / blockWidth;
                const u32 blocksY = (height + blockHeight - 1) / blockHeight;
                const DeviceSize rowBytes = PYRO_ALIGN(blocksX * 16, rowAlignment); // 16 bytes per block
                return rowBytes * blocksY * depth;
            }

            default: {
                // Uncompressed formats: width * bytesPerPixel
                const float bpp = RHIUtil::GetFormatSize(format);
                const DeviceSize rowBytes = PYRO_ALIGN(static_cast<DeviceSize>(width * bpp), static_cast<DeviceSize>(rowAlignment));
                return rowBytes * height * depth;
            }
            }
        }


        /**
         *  @brief Copies a single slice of a texture with row alignment in mind
         * The source data is tightly aligned, while the destination buffer must be row aligned, and large enough.
         * */
        static void CopyAlignedTextureData(const void* pSrc, void* pDst, u32 rowWidth, u32 height, u32 depth, u32 rowPitch) {
            const u8* srcPtr = reinterpret_cast<const u8*>(pSrc);
            u8* dstPtr = reinterpret_cast<u8*>(pDst);
            for (u32 z = 0; z < depth; ++z) {
                for (u32 y = 0; y < height; ++y) {
                    const u8* srcRow = srcPtr + z * height * rowWidth + y * rowWidth;
                    u8* dstRow = dstPtr + z * height * rowPitch + y * rowPitch;

                    memcpy(dstRow, srcRow, rowWidth);

                    // Optional: zero out padding (not strictly necessary)
                    // if (alignedRowSize > minRowSize) {
                    //    memset(dstRow + minRowSize, 0, alignedRowSize - minRowSize);
                    //}
                }
            }
        }

        /**
         *  @brief Returns true if the format is a depth-stencil format.
         * */
        static constexpr bool FormatIsDepthStencil(Format format) {
            switch (format) {
            case Format::S8Uint:
            case Format::D16Unorm:
            case Format::D32Sfloat:
            case Format::D16UnormS8Uint:
            case Format::D24UnormS8Uint:
            case Format::X8D24Unorm:
            case Format::D32SfloatS8Uint:
                return true;
            default:
                return false;
            }
        }
        /**
         * @brief Returns true if the format contains a stencil component.
         *  */
        static constexpr bool FormatHasStencil(Format format) {
            switch (format) {
            case Format::S8Uint:
            case Format::D16UnormS8Uint:
            case Format::D24UnormS8Uint:
            case Format::D32SfloatS8Uint:
                return true;
            default:
                return false;
            }
        }
        /**
         * @brief Returns true if the format contains a depth component.
         *  */
        static constexpr bool FormatHasDepth(Format format) {
            switch (format) {
            case Format::D16Unorm:
            case Format::D32Sfloat:
            case Format::D16UnormS8Uint:
            case Format::D24UnormS8Uint:
            case Format::X8D24Unorm:
            case Format::D32SfloatS8Uint:
                return true;
            default:
                return false;
            }
        }
    } // namespace RHIUtil
} // namespace PyroshockStudios