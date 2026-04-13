#include "gtest/gtest.h"
#include <PyroRHI/Api/Util.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::RHIUtil;
using namespace PyroshockStudios::Types;

TEST(FormatUtil, GetFormatNumericType_FloatFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R16Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RG16Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGB16Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA16Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::R32Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RG32Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGB32Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA32Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::D32Sfloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC6HUfloatBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC6HSfloatBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::B10GR11Ufloat), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::E5BGR9Ufloat), FormatNumericType::Float);
}

TEST(FormatUtil, GetFormatNumericType_UnormFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RG8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGB8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGR8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA8Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA4Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGR565Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGR5A1Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::A2RGB10Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::R16Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::D16Unorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::X8D24Unorm), FormatNumericType::Float);
}

TEST(FormatUtil, GetFormatNumericType_SnormFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RG8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGB8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGR8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA8Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::A2RGB10Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::R16Snorm), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC4SnormBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC5SnormBlock), FormatNumericType::Float);
}

TEST(FormatUtil, GetFormatNumericType_SrgbFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RG8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGB8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGR8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA8Srgb), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC1RGBSrgbBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC1RGBASrgbBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC2SrgbBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC3SrgbBlock), FormatNumericType::Float);
    EXPECT_EQ(GetFormatNumericType(Format::BC7SrgbBlock), FormatNumericType::Float);
}

TEST(FormatUtil, GetFormatNumericType_SignedIntFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RG8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGB8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::BGR8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA8Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::A2RGB10Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::R16Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::R32Sint), FormatNumericType::SignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA32Sint), FormatNumericType::SignedInt);
}

TEST(FormatUtil, GetFormatNumericType_UnsignedIntFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::R8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RG8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGB8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::BGR8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::BGRA8Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::A2RGB10Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::R16Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::R32Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::RGBA32Uint), FormatNumericType::UnsignedInt);
    EXPECT_EQ(GetFormatNumericType(Format::S8Uint), FormatNumericType::UnsignedInt);
}

TEST(FormatUtil, GetFormatNumericType_MixedFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::D32SfloatS8Uint), FormatNumericType::Mixed);
    EXPECT_EQ(GetFormatNumericType(Format::D16UnormS8Uint), FormatNumericType::Mixed);
    EXPECT_EQ(GetFormatNumericType(Format::D24UnormS8Uint), FormatNumericType::Mixed);
}

TEST(FormatUtil, GetFormatNumericType_OtherFormats) {
    EXPECT_EQ(GetFormatNumericType(Format::Undefined), FormatNumericType::Other);
    EXPECT_EQ(GetFormatNumericType(Format::Inherit), FormatNumericType::Other);
    EXPECT_EQ(GetFormatNumericType(static_cast<Format>(999)), FormatNumericType::Other);
}

TEST(FormatUtil, GetFormatComponentCount_OneComponent) {
    EXPECT_EQ(GetFormatComponentCount(Format::R8Unorm), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R8Snorm), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R8Uint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R8Sint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R8Srgb), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R16Unorm), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R16Sfloat), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R32Uint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R32Sint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::R32Sfloat), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::S8Uint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::D16Unorm), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::D32Sfloat), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::D16UnormS8Uint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::D24UnormS8Uint), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::X8D24Unorm), 1);
    EXPECT_EQ(GetFormatComponentCount(Format::D32SfloatS8Uint), 1);
}

TEST(FormatUtil, GetFormatComponentCount_TwoComponents) {
    EXPECT_EQ(GetFormatComponentCount(Format::RG8Unorm), 2);
    EXPECT_EQ(GetFormatComponentCount(Format::RG8Sint), 2);
    EXPECT_EQ(GetFormatComponentCount(Format::RG16Sfloat), 2);
    EXPECT_EQ(GetFormatComponentCount(Format::RG32Sfloat), 2);
    EXPECT_EQ(GetFormatComponentCount(Format::BC5UnormBlock), 2);
    EXPECT_EQ(GetFormatComponentCount(Format::BC5SnormBlock), 2);
}

TEST(FormatUtil, GetFormatComponentCount_ThreeComponents) {
    EXPECT_EQ(GetFormatComponentCount(Format::BGR565Unorm), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::RGB8Unorm), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::BGR8Unorm), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::RGB16Sfloat), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::RGB32Sfloat), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::BC1RGBUnormBlock), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::BC1RGBSrgbBlock), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::BC6HUfloatBlock), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::BC6HSfloatBlock), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::B10GR11Ufloat), 3);
    EXPECT_EQ(GetFormatComponentCount(Format::E5BGR9Ufloat), 3);
}

TEST(FormatUtil, GetFormatComponentCount_FourComponents) {
    EXPECT_EQ(GetFormatComponentCount(Format::BGRA4Unorm), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BGR5A1Unorm), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::RGBA8Unorm), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BGRA8Unorm), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::A2RGB10Unorm), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::RGBA16Sfloat), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::RGBA32Sfloat), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BC1RGBAUnormBlock), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BC2UnormBlock), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BC3UnormBlock), 4);
    EXPECT_EQ(GetFormatComponentCount(Format::BC7UnormBlock), 4);
}

TEST(FormatUtil, GetFormatComponentCount_UnknownReturnsZero) {
    EXPECT_EQ(GetFormatComponentCount(Format::Undefined), 0);
    EXPECT_EQ(GetFormatComponentCount(Format::Inherit), 0);
}

TEST(FormatUtil, GetFormatSize_8BitPerChannel) {
    EXPECT_EQ(GetFormatSize(Format::R8Unorm), 1u);
    EXPECT_EQ(GetFormatSize(Format::R8Snorm), 1u);
    EXPECT_EQ(GetFormatSize(Format::R8Uint), 1u);
    EXPECT_EQ(GetFormatSize(Format::R8Sint), 1u);
    EXPECT_EQ(GetFormatSize(Format::R8Srgb), 1u);

    EXPECT_EQ(GetFormatSize(Format::RG8Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::RG8Snorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::RG8Uint), 2u);
    EXPECT_EQ(GetFormatSize(Format::RG8Sint), 2u);
    EXPECT_EQ(GetFormatSize(Format::RG8Srgb), 2u);

    EXPECT_EQ(GetFormatSize(Format::RGB8Unorm), 3u);
    EXPECT_EQ(GetFormatSize(Format::RGB8Snorm), 3u);
    EXPECT_EQ(GetFormatSize(Format::BGR8Unorm), 3u);
    EXPECT_EQ(GetFormatSize(Format::BGR8Srgb), 3u);

    EXPECT_EQ(GetFormatSize(Format::RGBA8Unorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::RGBA8Snorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::BGRA8Unorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::BGRA8Srgb), 4u);
}

TEST(FormatUtil, GetFormatSize_16BitPerChannel) {
    EXPECT_EQ(GetFormatSize(Format::R16Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::R16Sfloat), 2u);
    EXPECT_EQ(GetFormatSize(Format::RG16Unorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::RG16Sfloat), 4u);
    EXPECT_EQ(GetFormatSize(Format::RGB16Unorm), 6u);
    EXPECT_EQ(GetFormatSize(Format::RGB16Sfloat), 6u);
    EXPECT_EQ(GetFormatSize(Format::RGBA16Unorm), 8u);
    EXPECT_EQ(GetFormatSize(Format::RGBA16Sfloat), 8u);
}

TEST(FormatUtil, GetFormatSize_32BitPerChannel) {
    EXPECT_EQ(GetFormatSize(Format::R32Uint), 4u);
    EXPECT_EQ(GetFormatSize(Format::R32Sfloat), 4u);
    EXPECT_EQ(GetFormatSize(Format::RG32Uint), 8u);
    EXPECT_EQ(GetFormatSize(Format::RG32Sfloat), 8u);
    EXPECT_EQ(GetFormatSize(Format::RGB32Uint), 12u);
    EXPECT_EQ(GetFormatSize(Format::RGB32Sfloat), 12u);
    EXPECT_EQ(GetFormatSize(Format::RGBA32Uint), 16u);
    EXPECT_EQ(GetFormatSize(Format::RGBA32Sfloat), 16u);
}

TEST(FormatUtil, GetFormatSize_PackedFormats) {
    EXPECT_EQ(GetFormatSize(Format::BGRA4Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::BGR565Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::BGR5A1Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::A2RGB10Unorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::A2RGB10Uint), 4u);
    EXPECT_EQ(GetFormatSize(Format::B10GR11Ufloat), 4u);
    EXPECT_EQ(GetFormatSize(Format::E5BGR9Ufloat), 4u);
}

TEST(FormatUtil, GetFormatSize_DepthStencil) {
    EXPECT_EQ(GetFormatSize(Format::D16Unorm), 2u);
    EXPECT_EQ(GetFormatSize(Format::X8D24Unorm), 4u);
    EXPECT_EQ(GetFormatSize(Format::D32Sfloat), 4u);
    EXPECT_EQ(GetFormatSize(Format::S8Uint), 1u);
    EXPECT_EQ(GetFormatSize(Format::D16UnormS8Uint), 3u);
    EXPECT_EQ(GetFormatSize(Format::D24UnormS8Uint), 4u);
    EXPECT_EQ(GetFormatSize(Format::D32SfloatS8Uint), 0u);
}

TEST(FormatUtil, GetFormatSize_BlockCompressed) {
    EXPECT_EQ(GetFormatSize(Format::BC1RGBUnormBlock), 8u);
    EXPECT_EQ(GetFormatSize(Format::BC1RGBSrgbBlock), 8u);
    EXPECT_EQ(GetFormatSize(Format::BC1RGBAUnormBlock), 8u);
    EXPECT_EQ(GetFormatSize(Format::BC4UnormBlock), 8u);
    EXPECT_EQ(GetFormatSize(Format::BC4SnormBlock), 8u);

    EXPECT_EQ(GetFormatSize(Format::BC2UnormBlock), 16u);
    EXPECT_EQ(GetFormatSize(Format::BC3UnormBlock), 16u);
    EXPECT_EQ(GetFormatSize(Format::BC5UnormBlock), 16u);
    EXPECT_EQ(GetFormatSize(Format::BC6HUfloatBlock), 16u);
    EXPECT_EQ(GetFormatSize(Format::BC7UnormBlock), 16u);
}

TEST(FormatUtil, GetFormatSize_UnknownReturnsZero) {
    EXPECT_EQ(GetFormatSize(Format::Undefined), 0u);
    EXPECT_EQ(GetFormatSize(Format::Inherit), 0u);
}

TEST(FormatUtil, GetFormatBlockInfo_StandardFormats) {
    auto info = GetFormatBlockInfo(Format::RGBA8Unorm);
    EXPECT_EQ(info.blockWidth, 1u);
    EXPECT_EQ(info.blockHeight, 1u);
    EXPECT_EQ(info.bytesPerBlock, 4u);

    auto info2 = GetFormatBlockInfo(Format::R32Sfloat);
    EXPECT_EQ(info2.blockWidth, 1u);
    EXPECT_EQ(info2.blockHeight, 1u);
    EXPECT_EQ(info2.bytesPerBlock, 4u);

    auto info3 = GetFormatBlockInfo(Format::D32SfloatS8Uint);
    EXPECT_EQ(info3.blockWidth, 1u);
    EXPECT_EQ(info3.blockHeight, 1u);
    EXPECT_EQ(info3.bytesPerBlock, 0u);
}

TEST(FormatUtil, GetFormatBlockInfo_BC8ByteFormats) {
    auto info = GetFormatBlockInfo(Format::BC1RGBUnormBlock);
    EXPECT_EQ(info.blockWidth, 4u);
    EXPECT_EQ(info.blockHeight, 4u);
    EXPECT_EQ(info.bytesPerBlock, 8u);

    auto info2 = GetFormatBlockInfo(Format::BC4SnormBlock);
    EXPECT_EQ(info2.blockWidth, 4u);
    EXPECT_EQ(info2.blockHeight, 4u);
    EXPECT_EQ(info2.bytesPerBlock, 8u);
}

TEST(FormatUtil, GetFormatBlockInfo_BC16ByteFormats) {
    auto info = GetFormatBlockInfo(Format::BC3UnormBlock);
    EXPECT_EQ(info.blockWidth, 4u);
    EXPECT_EQ(info.blockHeight, 4u);
    EXPECT_EQ(info.bytesPerBlock, 16u);

    auto info2 = GetFormatBlockInfo(Format::BC7SrgbBlock);
    EXPECT_EQ(info2.blockWidth, 4u);
    EXPECT_EQ(info2.blockHeight, 4u);
    EXPECT_EQ(info2.bytesPerBlock, 16u);

    auto info3 = GetFormatBlockInfo(Format::BC6HSfloatBlock);
    EXPECT_EQ(info3.blockWidth, 4u);
    EXPECT_EQ(info3.blockHeight, 4u);
    EXPECT_EQ(info3.bytesPerBlock, 16u);
}

TEST(FormatUtil, GetRequiredStagingSize_UncompressedRGBA8) {
    u32 rowAlignment = 256;
    DeviceSize size = GetRequiredStagingSize(Format::RGBA8Unorm, 4, 4, 1, rowAlignment);
    EXPECT_EQ(size, 256u * 4);

    size = GetRequiredStagingSize(Format::RGBA8Unorm, 64, 1, 1, rowAlignment);
    EXPECT_EQ(size, 256u);

    size = GetRequiredStagingSize(Format::RGBA8Unorm, 4, 4, 1, 1);
    EXPECT_EQ(size, 64u);
}

TEST(FormatUtil, GetRequiredStagingSize_UncompressedR32Sfloat) {
    u32 rowAlignment = 1;
    DeviceSize size = GetRequiredStagingSize(Format::R32Sfloat, 8, 8, 1, rowAlignment);
    EXPECT_EQ(size, 256u);

    rowAlignment = 256;
    size = GetRequiredStagingSize(Format::R32Sfloat, 8, 8, 1, rowAlignment);
    EXPECT_EQ(size, 256u * 8);
}

TEST(FormatUtil, GetRequiredStagingSize_BC1Format) {
    u32 rowAlignment = 1;
    DeviceSize size = GetRequiredStagingSize(Format::BC1RGBUnormBlock, 8, 8, 1, rowAlignment);
    EXPECT_EQ(size, 32u);

    size = GetRequiredStagingSize(Format::BC1RGBUnormBlock, 16, 16, 1, rowAlignment);
    EXPECT_EQ(size, 128u);

    size = GetRequiredStagingSize(Format::BC1RGBUnormBlock, 4, 4, 2, rowAlignment);
    EXPECT_EQ(size, 16u);

    rowAlignment = 256;
    size = GetRequiredStagingSize(Format::BC1RGBUnormBlock, 8, 8, 1, rowAlignment);
    EXPECT_EQ(size, 512u);
}

TEST(FormatUtil, GetRequiredStagingSize_BC3Format) {
    u32 rowAlignment = 1;
    DeviceSize size = GetRequiredStagingSize(Format::BC3UnormBlock, 8, 8, 1, rowAlignment);
    EXPECT_EQ(size, 64u);

    size = GetRequiredStagingSize(Format::BC3UnormBlock, 16, 16, 1, rowAlignment);
    EXPECT_EQ(size, 256u);
}

TEST(FormatUtil, GetRequiredStagingSize_DepthWithAlignment) {
    u32 rowAlignment = 256;
    DeviceSize size = GetRequiredStagingSize(Format::D32Sfloat, 4, 4, 1, rowAlignment);
    EXPECT_EQ(size, 256u * 4);
}

TEST(FormatUtil, FormatIsDepthStencil_TrueCases) {
    EXPECT_TRUE(FormatIsDepthStencil(Format::S8Uint));
    EXPECT_TRUE(FormatIsDepthStencil(Format::D16Unorm));
    EXPECT_TRUE(FormatIsDepthStencil(Format::D32Sfloat));
    EXPECT_TRUE(FormatIsDepthStencil(Format::D16UnormS8Uint));
    EXPECT_TRUE(FormatIsDepthStencil(Format::D24UnormS8Uint));
    EXPECT_TRUE(FormatIsDepthStencil(Format::X8D24Unorm));
    EXPECT_TRUE(FormatIsDepthStencil(Format::D32SfloatS8Uint));
}

TEST(FormatUtil, FormatIsDepthStencil_FalseCases) {
    EXPECT_FALSE(FormatIsDepthStencil(Format::Undefined));
    EXPECT_FALSE(FormatIsDepthStencil(Format::RGBA8Unorm));
    EXPECT_FALSE(FormatIsDepthStencil(Format::R32Sfloat));
    EXPECT_FALSE(FormatIsDepthStencil(Format::BGRA8Unorm));
    EXPECT_FALSE(FormatIsDepthStencil(Format::BC1RGBUnormBlock));
}

TEST(FormatUtil, FormatHasStencil_TrueCases) {
    EXPECT_TRUE(FormatHasStencil(Format::S8Uint));
    EXPECT_TRUE(FormatHasStencil(Format::D16UnormS8Uint));
    EXPECT_TRUE(FormatHasStencil(Format::D24UnormS8Uint));
    EXPECT_TRUE(FormatHasStencil(Format::D32SfloatS8Uint));
}

TEST(FormatUtil, FormatHasStencil_FalseCases) {
    EXPECT_FALSE(FormatHasStencil(Format::D16Unorm));
    EXPECT_FALSE(FormatHasStencil(Format::D32Sfloat));
    EXPECT_FALSE(FormatHasStencil(Format::X8D24Unorm));
    EXPECT_FALSE(FormatHasStencil(Format::RGBA8Unorm));
}

TEST(FormatUtil, FormatHasDepth_TrueCases) {
    EXPECT_TRUE(FormatHasDepth(Format::D16Unorm));
    EXPECT_TRUE(FormatHasDepth(Format::D32Sfloat));
    EXPECT_TRUE(FormatHasDepth(Format::D16UnormS8Uint));
    EXPECT_TRUE(FormatHasDepth(Format::D24UnormS8Uint));
    EXPECT_TRUE(FormatHasDepth(Format::X8D24Unorm));
    EXPECT_TRUE(FormatHasDepth(Format::D32SfloatS8Uint));
}

TEST(FormatUtil, FormatHasDepth_FalseCases) {
    EXPECT_FALSE(FormatHasDepth(Format::S8Uint));
    EXPECT_FALSE(FormatHasDepth(Format::RGBA8Unorm));
    EXPECT_FALSE(FormatHasDepth(Format::R32Sfloat));
}
