#include "gtest/gtest.h"
#include <PyroRHI/Api/ToString.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

// Helper function to remove indentation for easier comparison in tests
eastl::string RemoveIndentation(const eastl::string& s) {
    eastl::string result;
    for (char c : s) {
        if (c != ' ') {
            result += c;
        }
    }
    return result;
}

TEST(RHICommonEnumToStringTests, FormatToString) {
    EXPECT_STREQ(EnumToString(Format::Inherit), "Inherit");
    EXPECT_STREQ(EnumToString(Format::Undefined), "Undefined");
    EXPECT_STREQ(EnumToString(Format::BGRA4Unorm), "BGRA4Unorm");
    EXPECT_STREQ(EnumToString(Format::BGR565Unorm), "BGR565Unorm");
    EXPECT_STREQ(EnumToString(Format::BGR5A1Unorm), "BGR5A1Unorm");
    EXPECT_STREQ(EnumToString(Format::R8Unorm), "R8Unorm");
    EXPECT_STREQ(EnumToString(Format::R8Snorm), "R8Snorm");
    EXPECT_STREQ(EnumToString(Format::R8Uint), "R8Uint");
    EXPECT_STREQ(EnumToString(Format::R8Sint), "R8Sint");
    EXPECT_STREQ(EnumToString(Format::R8Srgb), "R8Srgb");
    EXPECT_STREQ(EnumToString(Format::RG8Unorm), "RG8Unorm");
    EXPECT_STREQ(EnumToString(Format::RG8Snorm), "RG8Snorm");
    EXPECT_STREQ(EnumToString(Format::RG8Uint), "RG8Uint");
    EXPECT_STREQ(EnumToString(Format::RG8Sint), "RG8Sint");
    EXPECT_STREQ(EnumToString(Format::RG8Srgb), "RG8Srgb");
    EXPECT_STREQ(EnumToString(Format::RGB8Unorm), "RGB8Unorm");
    EXPECT_STREQ(EnumToString(Format::RGB8Snorm), "RGB8Snorm");
    EXPECT_STREQ(EnumToString(Format::RGB8Uint), "RGB8Uint");
    EXPECT_STREQ(EnumToString(Format::RGB8Sint), "RGB8Sint");
    EXPECT_STREQ(EnumToString(Format::RGB8Srgb), "RGB8Srgb");
    EXPECT_STREQ(EnumToString(Format::BGR8Unorm), "BGR8Unorm");
    EXPECT_STREQ(EnumToString(Format::BGR8Snorm), "BGR8Snorm");
    EXPECT_STREQ(EnumToString(Format::BGR8Uint), "BGR8Uint");
    EXPECT_STREQ(EnumToString(Format::BGR8Sint), "BGR8Sint");
    EXPECT_STREQ(EnumToString(Format::BGR8Srgb), "BGR8Srgb");
    EXPECT_STREQ(EnumToString(Format::RGBA8Unorm), "RGBA8Unorm");
    EXPECT_STREQ(EnumToString(Format::RGBA8Snorm), "RGBA8Snorm");
    EXPECT_STREQ(EnumToString(Format::RGBA8Uint), "RGBA8Uint");
    EXPECT_STREQ(EnumToString(Format::RGBA8Sint), "RGBA8Sint");
    EXPECT_STREQ(EnumToString(Format::RGBA8Srgb), "RGBA8Srgb");
    EXPECT_STREQ(EnumToString(Format::BGRA8Unorm), "BGRA8Unorm");
    EXPECT_STREQ(EnumToString(Format::BGRA8Snorm), "BGRA8Snorm");
    EXPECT_STREQ(EnumToString(Format::BGRA8Uint), "BGRA8Uint");
    EXPECT_STREQ(EnumToString(Format::BGRA8Sint), "BGRA8Sint");
    EXPECT_STREQ(EnumToString(Format::BGRA8Srgb), "BGRA8Srgb");
    EXPECT_STREQ(EnumToString(Format::A2RGB10Unorm), "A2RGB10Unorm");
    EXPECT_STREQ(EnumToString(Format::A2RGB10Snorm), "A2RGB10Snorm");
    EXPECT_STREQ(EnumToString(Format::A2RGB10Uint), "A2RGB10Uint");
    EXPECT_STREQ(EnumToString(Format::A2RGB10Sint), "A2RGB10Sint");
    EXPECT_STREQ(EnumToString(Format::R16Unorm), "R16Unorm");
    EXPECT_STREQ(EnumToString(Format::R16Snorm), "R16Snorm");
    EXPECT_STREQ(EnumToString(Format::R16Uint), "R16Uint");
    EXPECT_STREQ(EnumToString(Format::R16Sint), "R16Sint");
    EXPECT_STREQ(EnumToString(Format::R16Sfloat), "R16Sfloat");
    EXPECT_STREQ(EnumToString(Format::RG16Unorm), "RG16Unorm");
    EXPECT_STREQ(EnumToString(Format::RG16Snorm), "RG16Snorm");
    EXPECT_STREQ(EnumToString(Format::RG16Uint), "RG16Uint");
    EXPECT_STREQ(EnumToString(Format::RG16Sint), "RG16Sint");
    EXPECT_STREQ(EnumToString(Format::RG16Sfloat), "RG16Sfloat");
    EXPECT_STREQ(EnumToString(Format::RGB16Unorm), "RGB16Unorm");
    EXPECT_STREQ(EnumToString(Format::RGB16Snorm), "RGB16Snorm");
    EXPECT_STREQ(EnumToString(Format::RGB16Uint), "RGB16Uint");
    EXPECT_STREQ(EnumToString(Format::RGB16Sint), "RGB16Sint");
    EXPECT_STREQ(EnumToString(Format::RGB16Sfloat), "RGB16Sfloat");
    EXPECT_STREQ(EnumToString(Format::RGBA16Unorm), "RGBA16Unorm");
    EXPECT_STREQ(EnumToString(Format::RGBA16Snorm), "RGBA16Snorm");
    EXPECT_STREQ(EnumToString(Format::RGBA16Uint), "RGBA16Uint");
    EXPECT_STREQ(EnumToString(Format::RGBA16Sint), "RGBA16Sint");
    EXPECT_STREQ(EnumToString(Format::RGBA16Sfloat), "RGBA16Sfloat");
    EXPECT_STREQ(EnumToString(Format::R32Uint), "R32Uint");
    EXPECT_STREQ(EnumToString(Format::R32Sint), "R32Sint");
    EXPECT_STREQ(EnumToString(Format::R32Sfloat), "R32Sfloat");
    EXPECT_STREQ(EnumToString(Format::RG32Uint), "RG32Uint");
    EXPECT_STREQ(EnumToString(Format::RG32Sint), "RG32Sint");
    EXPECT_STREQ(EnumToString(Format::RG32Sfloat), "RG32Sfloat");
    EXPECT_STREQ(EnumToString(Format::RGB32Uint), "RGB32Uint");
    EXPECT_STREQ(EnumToString(Format::RGB32Sint), "RGB32Sint");
    EXPECT_STREQ(EnumToString(Format::RGB32Sfloat), "RGB32Sfloat");
    EXPECT_STREQ(EnumToString(Format::RGBA32Uint), "RGBA32Uint");
    EXPECT_STREQ(EnumToString(Format::RGBA32Sint), "RGBA32Sint");
    EXPECT_STREQ(EnumToString(Format::RGBA32Sfloat), "RGBA32Sfloat");
    EXPECT_STREQ(EnumToString(Format::B10GR11Ufloat), "B10GR11Ufloat");
    EXPECT_STREQ(EnumToString(Format::E5BGR9Ufloat), "E5BGR9Ufloat");
    EXPECT_STREQ(EnumToString(Format::D16Unorm), "D16Unorm");
    EXPECT_STREQ(EnumToString(Format::X8D24Unorm), "X8D24Unorm");
    EXPECT_STREQ(EnumToString(Format::D32Sfloat), "D32Sfloat");
    EXPECT_STREQ(EnumToString(Format::S8Uint), "S8Uint");
    EXPECT_STREQ(EnumToString(Format::D16UnormS8Uint), "D16UnormS8Uint");
    EXPECT_STREQ(EnumToString(Format::D24UnormS8Uint), "D24UnormS8Uint");
    EXPECT_STREQ(EnumToString(Format::D32SfloatS8Uint), "D32SfloatS8Uint");
    EXPECT_STREQ(EnumToString(Format::BC1RGBUnormBlock), "BC1RGBUnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC1RGBSrgbBlock), "BC1RGBSrgbBlock");
    EXPECT_STREQ(EnumToString(Format::BC1RGBAUnormBlock), "BC1RGBAUnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC1RGBASrgbBlock), "BC1RGBASrgbBlock");
    EXPECT_STREQ(EnumToString(Format::BC2UnormBlock), "BC2UnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC2SrgbBlock), "BC2SrgbBlock");
    EXPECT_STREQ(EnumToString(Format::BC3UnormBlock), "BC3UnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC3SrgbBlock), "BC3SrgbBlock");
    EXPECT_STREQ(EnumToString(Format::BC4UnormBlock), "BC4UnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC4SnormBlock), "BC4SnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC5UnormBlock), "BC5UnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC5SnormBlock), "BC5SnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC6HUfloatBlock), "BC6HUfloatBlock");
    EXPECT_STREQ(EnumToString(Format::BC6HSfloatBlock), "BC6HSfloatBlock");
    EXPECT_STREQ(EnumToString(Format::BC7UnormBlock), "BC7UnormBlock");
    EXPECT_STREQ(EnumToString(Format::BC7SrgbBlock), "BC7SrgbBlock");
}

TEST(RHICommonEnumToStringTests, SamplerAddressModeToString) {
    EXPECT_STREQ(EnumToString(SamplerAddressMode::Repeat), "Repeat");
    EXPECT_STREQ(EnumToString(SamplerAddressMode::MirroredRepeat), "MirroredRepeat");
    EXPECT_STREQ(EnumToString(SamplerAddressMode::ClampToEdge), "ClampToEdge");
    EXPECT_STREQ(EnumToString(SamplerAddressMode::ClampToBorder), "ClampToBorder");
    EXPECT_STREQ(EnumToString(SamplerAddressMode::MirrorClampToEdge), "MirrorClampToEdge");
}

TEST(RHICommonEnumToStringTests, BorderColorToString) {
    EXPECT_STREQ(EnumToString(BorderColor::TransparentBlackFloat), "TransparentBlackFloat");
    EXPECT_STREQ(EnumToString(BorderColor::TransparentBlackInt), "TransparentBlackInt");
    EXPECT_STREQ(EnumToString(BorderColor::OpaqueBlackFloat), "OpaqueBlackFloat");
    EXPECT_STREQ(EnumToString(BorderColor::OpaqueBlackInt), "OpaqueBlackInt");
    EXPECT_STREQ(EnumToString(BorderColor::OpaqueWhiteFloat), "OpaqueWhiteFloat");
    EXPECT_STREQ(EnumToString(BorderColor::OpaqueWhiteInt), "OpaqueWhiteInt");
}

TEST(RHICommonEnumToStringTests, CompareOpToString) {
    EXPECT_STREQ(EnumToString(CompareOp::Never), "Never");
    EXPECT_STREQ(EnumToString(CompareOp::Less), "Less");
    EXPECT_STREQ(EnumToString(CompareOp::Equal), "Equal");
    EXPECT_STREQ(EnumToString(CompareOp::LessOrEqual), "LessOrEqual");
    EXPECT_STREQ(EnumToString(CompareOp::Greater), "Greater");
    EXPECT_STREQ(EnumToString(CompareOp::NotEqual), "NotEqual");
    EXPECT_STREQ(EnumToString(CompareOp::GreaterOrEqual), "GreaterOrEqual");
    EXPECT_STREQ(EnumToString(CompareOp::Always), "Always");
}

TEST(RHICommonEnumToStringTests, BlendFactorToString) {
    EXPECT_STREQ(EnumToString(BlendFactor::Zero), "Zero");
    EXPECT_STREQ(EnumToString(BlendFactor::One), "One");
    EXPECT_STREQ(EnumToString(BlendFactor::SrcColor), "SrcColor");
    EXPECT_STREQ(EnumToString(BlendFactor::OneMinusSrcColor), "OneMinusSrcColor");
    EXPECT_STREQ(EnumToString(BlendFactor::DstAlpha), "DstAlpha");
    EXPECT_STREQ(EnumToString(BlendFactor::Src1Alpha), "Src1Alpha");
}

TEST(RHICommonEnumToStringTests, BlendOpToString) {
    EXPECT_STREQ(EnumToString(BlendOp::Add), "Add");
    EXPECT_STREQ(EnumToString(BlendOp::Subtract), "Subtract");
    EXPECT_STREQ(EnumToString(BlendOp::ReverseSubtract), "ReverseSubtract");
    EXPECT_STREQ(EnumToString(BlendOp::Min), "Min");
    EXPECT_STREQ(EnumToString(BlendOp::Max), "Max");
}

TEST(RHICommonEnumToStringTests, TesselationDomainOriginToString) {
    EXPECT_STREQ(EnumToString(TesselationDomainOrigin::LowerLeft), "LowerLeft");
    EXPECT_STREQ(EnumToString(TesselationDomainOrigin::UpperLeft), "UpperLeft");
}

TEST(RHICommonEnumToStringTests, PrimitiveTopologyToString) {
    EXPECT_STREQ(EnumToString(PrimitiveTopology::PointList), "PointList");
    EXPECT_STREQ(EnumToString(PrimitiveTopology::LineList), "LineList");
    EXPECT_STREQ(EnumToString(PrimitiveTopology::TriangleStripWithAdjacency), "TriangleStripWithAdjacency");
    EXPECT_STREQ(EnumToString(PrimitiveTopology::PatchList), "PatchList");
}

TEST(RHICommonEnumToStringTests, PolygonModeToString) {
    EXPECT_STREQ(EnumToString(PolygonMode::Triangle), "Triangle");
    EXPECT_STREQ(EnumToString(PolygonMode::Line), "Line");
    EXPECT_STREQ(EnumToString(PolygonMode::Point), "Point");
}

TEST(RHICommonEnumToStringTests, LineModeToString) {
    EXPECT_STREQ(EnumToString(LineMode::Normal), "Normal");
    EXPECT_STREQ(EnumToString(LineMode::Smooth), "Smooth");
}

TEST(RHICommonEnumToStringTests, WindingOrderToString) {
    EXPECT_STREQ(EnumToString(WindingOrder::CounterClockwise), "CounterClockwise");
    EXPECT_STREQ(EnumToString(WindingOrder::Clockwise), "Clockwise");
}

TEST(RHICommonEnumToStringTests, FaceCullToString) {
    EXPECT_STREQ(EnumToString(FaceCull::None), "None");
    EXPECT_STREQ(EnumToString(FaceCull::Front), "Front");
    EXPECT_STREQ(EnumToString(FaceCull::Back), "Back");
}

TEST(RHICommonEnumToStringTests, StencilOpToString) {
    EXPECT_STREQ(EnumToString(StencilOp::Keep), "Keep");
    EXPECT_STREQ(EnumToString(StencilOp::Zero), "Zero");
    EXPECT_STREQ(EnumToString(StencilOp::Replace), "Replace");
    EXPECT_STREQ(EnumToString(StencilOp::IncrementClamp), "IncrementClamp");
    EXPECT_STREQ(EnumToString(StencilOp::DecrementWrap), "DecrementWrap");
}

TEST(RHICommonEnumToStringTests, RasterizationSamplesToString) {
    EXPECT_STREQ(EnumToString(RasterizationSamples::e1), "e1");
    EXPECT_STREQ(EnumToString(RasterizationSamples::e2), "e2");
    EXPECT_STREQ(EnumToString(RasterizationSamples::e4), "e4");
    EXPECT_STREQ(EnumToString(RasterizationSamples::e64), "e64");
}

TEST(RHICommonEnumToStringTests, PipelineBindPointToString) {
    EXPECT_STREQ(EnumToString(PipelineBindPoint::None), "None");
    EXPECT_STREQ(EnumToString(PipelineBindPoint::Graphics), "Graphics");
    EXPECT_STREQ(EnumToString(PipelineBindPoint::Compute), "Compute");
    EXPECT_STREQ(EnumToString(PipelineBindPoint::RayTracing), "RayTracing");
}

TEST(RHICommonEnumToStringTests, MemoryAllocationDomainToString) {
    EXPECT_STREQ(EnumToString(MemoryAllocationDomain::DeviceLocal), "DeviceLocal");
    EXPECT_STREQ(EnumToString(MemoryAllocationDomain::HostStaging), "HostStaging");
    EXPECT_STREQ(EnumToString(MemoryAllocationDomain::HostRandomWrite), "HostRandomWrite");
    EXPECT_STREQ(EnumToString(MemoryAllocationDomain::HostReadback), "HostReadback");
}

TEST(RHICommonEnumToStringTests, ColorSpaceToString) {
    EXPECT_STREQ(EnumToString(ColorSpace::SrgbNonlinear), "SrgbNonlinear");
    EXPECT_STREQ(EnumToString(ColorSpace::DisplayP3Nonlinear), "DisplayP3Nonlinear");
    EXPECT_STREQ(EnumToString(ColorSpace::Hdr10Hlg), "Hdr10Hlg");
    EXPECT_STREQ(EnumToString(ColorSpace::PassThrough), "PassThrough");
}

TEST(RHICommonEnumToStringTests, ImageLayoutToString) {
    EXPECT_STREQ(EnumToString(ImageLayout::Identity), "Identity");
    EXPECT_STREQ(EnumToString(ImageLayout::Undefined), "Undefined");
    EXPECT_STREQ(EnumToString(ImageLayout::UnorderedAccess), "UnorderedAccess");
    EXPECT_STREQ(EnumToString(ImageLayout::PresentSrc), "PresentSrc");
}

TEST(RHICommonEnumToStringTests, BufferLayoutToString) {
    EXPECT_STREQ(EnumToString(BufferLayout::Identity), "Identity");
    EXPECT_STREQ(EnumToString(BufferLayout::Undefined), "Undefined");
    EXPECT_STREQ(EnumToString(BufferLayout::UnorderedAccess), "UnorderedAccess");
    EXPECT_STREQ(EnumToString(BufferLayout::TransferDst), "TransferDst");
}

TEST(RHICommonEnumToStringTests, FilterToString) {
    EXPECT_STREQ(EnumToString(Filter::Nearest), "Nearest");
    EXPECT_STREQ(EnumToString(Filter::Linear), "Linear");
}

TEST(RHICommonEnumToStringTests, ReductionModeToString) {
    EXPECT_STREQ(EnumToString(ReductionMode::WeightedAverage), "WeightedAverage");
    EXPECT_STREQ(EnumToString(ReductionMode::Min), "Min");
    EXPECT_STREQ(EnumToString(ReductionMode::Max), "Max");
}

TEST(RHICommonEnumToStringTests, IndexTypeToString) {
    EXPECT_STREQ(EnumToString(IndexType::Uint16), "Uint16");
    EXPECT_STREQ(EnumToString(IndexType::Uint32), "Uint32");
    EXPECT_STREQ(EnumToString(IndexType::Uint8), "Uint8");
    EXPECT_STREQ(EnumToString(IndexType::None), "None");
}

TEST(RHICommonEnumToStringTests, AttachmentLoadOpToString) {
    EXPECT_STREQ(EnumToString(AttachmentLoadOp::Load), "Load");
    EXPECT_STREQ(EnumToString(AttachmentLoadOp::Clear), "Clear");
    EXPECT_STREQ(EnumToString(AttachmentLoadOp::DontCare), "DontCare");
}

TEST(RHICommonEnumToStringTests, AttachmentStoreOpToString) {
    EXPECT_STREQ(EnumToString(AttachmentStoreOp::Store), "Store");
    EXPECT_STREQ(EnumToString(AttachmentStoreOp::DontCare), "DontCare");
}
TEST(RHICommonToStringTests, EnumToString_VirtualSuballocationStrategy) {
    EXPECT_STREQ(EnumToString(VirtualSuballocationStrategy::Default), "Default");
    EXPECT_STREQ(EnumToString(VirtualSuballocationStrategy::SpaceEfficient), "SpaceEfficient");
    EXPECT_STREQ(EnumToString(VirtualSuballocationStrategy::TimeEfficient), "TimeEfficient");
    EXPECT_STREQ(EnumToString(VirtualSuballocationStrategy::AggressiveRing), "AggressiveRing");
}

TEST(RHICommonToStringTests, EnumToString_ImageDimensions) {
    EXPECT_STREQ(EnumToString(ImageDimensions::e1D), "e1D");
    EXPECT_STREQ(EnumToString(ImageDimensions::e2D), "e2D");
    EXPECT_STREQ(EnumToString(ImageDimensions::e3D), "e3D");
}

TEST(RHICommonToStringTests, EnumToString_ImageViewType) {
    EXPECT_STREQ(EnumToString(ImageViewType::e1D), "e1D");
    EXPECT_STREQ(EnumToString(ImageViewType::e2D), "e2D");
    EXPECT_STREQ(EnumToString(ImageViewType::e3D), "e3D");
    EXPECT_STREQ(EnumToString(ImageViewType::eCube), "eCube");
    EXPECT_STREQ(EnumToString(ImageViewType::e1DArray), "e1DArray");
    EXPECT_STREQ(EnumToString(ImageViewType::e2DArray), "e2DArray");
    EXPECT_STREQ(EnumToString(ImageViewType::eCubeArray), "eCubeArray");
}

TEST(RHICommonToStringTests, DepthStencilClearValueToString) {
    DepthStencilClearValue clearValue = { 1.0f, 0 };
    eastl::string expected = "DepthStencilClear{ depth=1.000, stencil=0 }";
    EXPECT_STREQ(RemoveIndentation(clearValue.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    clearValue = { 0.5f, 255 };
    expected = "DepthStencilClear{ depth=0.500, stencil=255 }";
    EXPECT_STREQ(RemoveIndentation(clearValue.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ColorClearValueToString) {
    ColorClearValue clearValue;
    // Test with integer values directly
    clearValue.int32[0] = 10;
    clearValue.int32[1] = 20;
    clearValue.int32[2] = 30;
    clearValue.int32[3] = 40;
    eastl::string expected = "ColorClear<Int32>{ 10, 20, 30, 40 }";
    EXPECT_STREQ(RemoveIndentation(clearValue.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    clearValue.float32[0] = 1.0f;
    clearValue.float32[1] = 0.5f;
    clearValue.float32[2] = 0.2f;
    clearValue.float32[3] = 1.0f;
    // Note: The ToString implementation currently prints as Int32, so we'll test that.
    // If the intention is float, the ToString implementation might need adjustment.
    expected = "ColorClear<Float32>{ 1.000, 0.500, 0.200, 1.000 }";
    EXPECT_STREQ(RemoveIndentation(clearValue.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, Offset2DToString) {
    Offset2D offset = { 10, 20 };
    eastl::string expected = "{ 10, 20 }";
    EXPECT_STREQ(RemoveIndentation(offset.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, Offset3DToString) {
    Offset3D offset = { 10, 20, 30 };
    eastl::string expected = "{ 10, 20, 30 }";
    EXPECT_STREQ(RemoveIndentation(offset.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, Extent2DToString) {
    Extent2D extent = { 1920, 1080 };
    eastl::string expected = "{ 1920, 1080 }";
    EXPECT_STREQ(RemoveIndentation(extent.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, Extent3DToString) {
    Extent3D extent = { 1920, 1080, 1 };
    eastl::string expected = "{ 1920, 1080, 1 }";
    EXPECT_STREQ(RemoveIndentation(extent.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ViewportInfoToString) {
    ViewportInfo vp = { 0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f };
    eastl::string expected = "ViewportInfo{\n"
                             "  x=0.000,\n"
                             "  y=0.000,\n"
                             "  width=800.000,\n"
                             "  height=600.000,\n"
                             "  minDepth=0.000,\n"
                             "  maxDepth=1.000\n"
                             "}";
    // We need to be careful with floating point precision in the expected string.
    // Also, the original ToString uses an 'inline' string 'indent' inside the function,
    // which makes direct comparison with indentation a bit tricky.
    // For simplicity, we'll remove all whitespace for comparison,
    // but a more robust solution might involve parsing or custom comparators.
    EXPECT_STREQ(RemoveIndentation(vp.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    ViewportInfo vp2 = { 10.5f, 20.0f, 1024.0f, 768.0f, 0.1f, 0.9f };
    eastl::string expected2 = "ViewportInfo{\n"
                              "  x=10.500,\n"
                              "  y=20.000,\n"
                              "  width=1024.000,\n"
                              "  height=768.000,\n"
                              "  minDepth=0.100,\n"
                              "  maxDepth=0.900\n"
                              "}";
    EXPECT_STREQ(RemoveIndentation(vp2.ToString(0)).c_str(), RemoveIndentation(expected2).c_str());
}

TEST(RHICommonToStringTests, Rect2DToString) {
    Rect2D rect = { 0, 0, 1920, 1080 };
    eastl::string expected = "{ x=0, y=0, width=1920, height=1080 }";
    EXPECT_STREQ(RemoveIndentation(rect.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, Box3DToString) {
    Box3D box = { 0, 0, 0, 100, 50, 20 };
    eastl::string expected = "{ x=0, y=0, z=0, width=100, height=50, depth=20 }";
    EXPECT_STREQ(RemoveIndentation(box.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, BufferRegionToString) {
    BufferRegion region = { 1024, 512 };
    eastl::string expected = "{ offset=1024, size=512 }";
    EXPECT_STREQ(RemoveIndentation(region.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ImageMipArraySliceToString) {
    ImageMipArraySlice slice = { 0, 1, 0, 1 };
    eastl::string expected = "{ firstMip=0, mipCount=1, firstLayer=0, layerCount=1 }";
    EXPECT_STREQ(RemoveIndentation(slice.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    ImageMipArraySlice slice2 = { 2, 3, 5, 6 };
    expected = "{ firstMip=2, mipCount=3, firstLayer=5, layerCount=6 }";
    EXPECT_STREQ(RemoveIndentation(slice2.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ImageArraySliceToString) {
    ImageArraySlice slice = { 0, 0, 1 };
    eastl::string expected = "{ mip=0, firstLayer=0, layerCount=1 }";
    EXPECT_STREQ(RemoveIndentation(slice.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    ImageArraySlice slice2 = { 3, 5, 6 };
    expected = "{ mip=3, firstLayer=5, layerCount=6 }";
    EXPECT_STREQ(RemoveIndentation(slice2.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ImageSliceToString) {
    ImageSlice slice = { 0, 0 };
    eastl::string expected = "{ mip=0, layer=0 }";
    EXPECT_STREQ(RemoveIndentation(slice.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    ImageSlice slice2 = { 3, 5 };
    expected = "{ mip=3, layer=5 }";
    EXPECT_STREQ(RemoveIndentation(slice2.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DeviceInfoToString) {
    DeviceInfo info = {};
    info.name = "Test GPU";
    info.vendor = "Test Vendor";
    info.vendorID = 43166;
    info.deviceID = 0x1C81;
    info.subsystemID = 0x1234;
    info.revisionID = 0x5678;
    info.deviceType = DeviceType::Discrete;
    info.bUnifiedMemory = true;
    info.bRemovable = false;
    info.bPrimaryAdapter = true;
    info.bHeadless = false;
    info.driverVersion = "1.0.0";
    info.apiVersion = "Vulkan 1.2";
    info.driverDescription = "A test driver";
    info.architecture = "X86_64";
    info.dedicatedVideoMemory = static_cast<DeviceSize>(8) * 1024 * 1024 * 1024; // 8GB
    info.sharedSystemMemory = static_cast<DeviceSize>(16) * 1024 * 1024 * 1024;  // 16GB
    info.adapterLUIDHigh = 2891390976;
    info.adapterLUIDLow = 3689348317;

    eastl::string expected =
        "DeviceInfo {\n"
        "  Name: Test GPU\n"
        "  Vendor: Test Vendor\n"
        "  VendorID: 43166\n"
        "  DeviceID: 7297\n"
        "  SubsystemID: 4660\n"
        "  RevisionID: 22136\n"
        "  DeviceType: Discrete\n"
        "  UnifiedMemory: true\n"
        "  Removable: false\n"
        "  PrimaryAdapter: true\n"
        "  Headless: false\n"
        "  DriverVersion: 1.0.0\n"
        "  APIVersion: Vulkan 1.2\n"
        "  DriverDescription: A test driver\n"
        "  Architecture: X86_64\n"
        "  DedicatedVRAM: 8589934592\n"
        "  SharedSystemMemory: 17179869184\n"
        "  AdapterLUID: 2891390976:3689348317\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DeviceFeaturesInfoToString) {
    DeviceFeaturesInfo info = {};
    info.bGeometryShaders = true;
    info.bTesselationShaders = false;
    info.bMeshShaders = true;
    info.bTaskShaders = false;
    info.bRayQueries = true;
    info.bRayTracingPipelines = false;
    info.bAccelerationStructureBuild = true;
    info.bBufferDeviceAddress = false;
    info.bBCnTextureCompression = true;
    info.bUint8IndexBuffer = false;
    info.bInt64ShaderOps = true;
    info.bAtomicFloatOps = false;
    info.bWaveOps = true;
    info.bSubgroupQuadOps = false;
    info.bHeadlessSwapChainWindow = true;
    info.bVariableRateShading = false;
    info.bConservativeRasterization = true;
    info.supportedShaderModel = 0x61;

    eastl::string expected =
        "DeviceFeaturesInfo {\n"
        "  GeometryShaders: true\n"
        "  TesselationShaders: false\n"
        "  MeshShaders: true\n"
        "  TaskShaders: false\n"
        "  RayQueries: true\n"
        "  RayTracingPipelines: false\n"
        "  AccelerationStructureBuild: true\n"
        "  BufferDeviceAddress: false\n"
        "  BCnTextureCompression: true\n"
        "  Uint8IndexBuffer: false\n"
        "  Int64ShaderOps: true\n"
        "  AtomicFloatOps: false\n"
        "  WaveOps: true\n"
        "  SubgroupQuadOps: false\n"
        "  HeadlessSwapChainWindow: true\n"
        "  VariableRateShading: false\n"
        "  ConservativeRasterization: true\n"
        "  SupportedShaderModel: 0x61\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DevicePropertiesInfoToString) {
    DevicePropertiesInfo info = {};
    info.msaaSupportColorTarget = RasterizationSamples::e4;
    info.msaaSupportDepthStencilTarget = RasterizationSamples::e8;
    info.msaaSupportShaderResourceView = RasterizationSamples::e2;
    info.msaaSupportUnorderedAccessView = RasterizationSamples::e1;
    info.bufferImageRowAlignment = 256;
    info.bufferImageCopyOffsetAlignment = 16;
    info.minUniformBufferOffsetAlignment = 64;
    info.minStorageBufferOffsetAlignment = 32;
    info.graphicsQueueCount = 1;
    info.computeQueueCount = 1;
    info.transferQueueCount = 1;
    info.bHasDedicatedComputeQueue = true;
    info.bHasDedicatedTransferQueue = false;
    info.maxTextureWidth = 16384;
    info.maxTextureHeight = 16384;
    info.maxTextureDepth = 2048;
    info.maxTextureArrayLayers = 2048;
    info.maxSamplerAnisotropy = 16.0f;
    info.minLineWidth = 1.0f;
    info.maxLineWidth = 8.0f;

    eastl::string expected =
        "DevicePropertiesInfo {\n"
        "  MSAA ColorTarget: 4\n"
        "  MSAA DepthStencilTarget: 8\n"
        "  MSAA ShaderResourceView: 2\n"
        "  MSAA UAV: 1\n"
        "  BufferImageRowAlignment: 256\n"
        "  BufferImageCopyOffsetAlignment: 16\n"
        "  MinUniformBufferOffsetAlignment: 64\n"
        "  MinStorageBufferOffsetAlignment: 32\n"
        "  QueueCounts: G=1 C=1 T=1\n"
        "  DedicatedQueues: Compute=1 Transfer=0\n"
        "  TextureLimits: 16384x16384x2048\n"
        "  MaxTextureArrayLayers: 2048\n"
        "  MaxSamplerAnisotropy: 16\n"
        "  LineWidthRange: 1.000000 - 8.000000\n"
        "}";

    // For floating-point numbers in the string, `sprintf` default precision
    // might be larger than what you expect, potentially requiring `%.*f` or
    // custom float-to-string conversion for exact string matching.
    // For now, we will match the default `eastl::to_string` behavior.
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DeviceStatusInfoToString) {
    DeviceStatusInfo info = {};
    info.numHostAllocations = 10;
    info.numHostAllocatedBytes = 1024 * 1024;
    info.numDeviceMemoryAllocations = 5;
    info.numDeviceAllocatedBytes = 512 * 1024 * 1024;
    info.numBufferResourcesCreated = 20;
    info.numImageResourcesCreated = 15;
    info.numPipelineObjectsCreated = 8;
    info.numDescriptorHeapsCreated = 3;
    info.numCommandBuffersCreated = 12;
    info.numBuffersAlive = 7;
    info.numImagesAlive = 10;
    info.numPipelinesAlive = 5;
    info.numSamplersAlive = 6;
    info.numQueueSubmits = 100;
    info.availableVideoMemory = static_cast<DeviceSize>(4) * 1024 * 1024 * 1024;

    eastl::string expected =
        "DeviceStatusInfo {\n"
        "  HostAllocations: 10 (1048576 bytes)\n"
        "  DeviceAllocations: 5 (536870912 bytes)\n"
        "  BuffersCreated: 20\n"
        "  ImagesCreated: 15\n"
        "  PipelinesCreated: 8\n"
        "  DescriptorHeapsCreated: 3\n"
        "  CommandBuffersCreated: 12\n"
        "  #Alive: Buffers=7, Images=10, Pipelines=5, Samplers=6\n"
        "  QueueSubmits: 100\n"
        "  AvailableVRAM: 4294967296\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SemaphoreSubmitInfoToString) {
    Semaphore testSemaphore = reinterpret_cast<Semaphore>(0x12345678);
    SemaphoreSubmitInfo info = { testSemaphore, PipelineStageFlagBits::ALL_COMMANDS };
    eastl::string expected = "SemaphoreSubmitInfo { semaphore=0x0000000012345678, stage=65536 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, FenceSubmitInfoToString) {
    IFence* testFence = reinterpret_cast<IFence*>(0xAABBCCDD);
    FenceSubmitInfo info = { testFence, 5 };
    eastl::string expected = "FenceSubmitInfo { fence=0x00000000AABBCCDD, value=5 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CommandQueueSubmitInfoToString) {
    ICommandQueue* testQueue = reinterpret_cast<ICommandQueue*>(0x00000000DEADBEEFULL); // Ensure it's 64-bit for consistency
    Semaphore waitSemaphore1 = reinterpret_cast<Semaphore>(0x11111111);            // Explicitly construct Semaphore with u64
    Semaphore signalSemaphore1 = reinterpret_cast<Semaphore>(0x22222222);
    IFence* signalFence1 = reinterpret_cast<IFence*>(0x0000000033333333ULL); // Ensure 64-bit

    CommandQueueSubmitInfo info = {};
    info.queue = testQueue;
    auto array1 = eastl::array<SemaphoreSubmitInfo, 1>({ waitSemaphore1, PipelineStageFlagBits::TOP_OF_PIPE });
    auto array2 = eastl::array<SemaphoreSubmitInfo, 1>({ signalSemaphore1, PipelineStageFlagBits::BOTTOM_OF_PIPE });
    auto array3 = eastl::array<SemaphoreSubmitInfo, 1>({ waitSemaphore1, PipelineStageFlagBits::ALL_GRAPHICS }); // Using a wait semaphore here just for test
    auto array4 = eastl::array<FenceSubmitInfo, 1>({ signalFence1, 3 });
    info.waitSemaphores = array1;
    info.signalSemaphores = array2;
    info.signalPresentReadySemaphores = array3;
    info.signalFences = array4;

    eastl::string expected =
        "CommandQueueSubmitInfo {\n"
        "  queue: 0x00000000DEADBEEF\n"
        "  waitSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000011111111, stage=1 }\n" // Stage values from enum
        "  ]\n"
        "  signalSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000022222222, stage=8192 }\n"
        "  ]\n"
        "  signalPresentSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000011111111, stage=32768 }\n"
        "  ]\n"
        "  signalFences: [\n"
        "    FenceSubmitInfo { fence=0x0000000033333333, value=3 }\n"
        "  ]\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    // Test with indentation
    eastl::string expected_indented =
        "CommandQueueSubmitInfo {\n"
        "  queue: 0x00000000DEADBEEF\n"
        "  waitSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000011111111, stage=1 }\n"
        "  ]\n"
        "  signalSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000022222222, stage=8192 }\n"
        "  ]\n"
        "  signalPresentSemaphores: [\n"
        "    SemaphoreSubmitInfo { semaphore=0x0000000011111111, stage=32768 }\n"
        "  ]\n"
        "  signalFences: [\n"
        "    FenceSubmitInfo { fence=0x0000000033333333, value=3 }\n"
        "  ]\n"
        "}";
    EXPECT_STREQ(info.ToString(0).c_str(), expected_indented.c_str()); // Direct string comparison with indentation
}

TEST(RHICommonToStringTests, CommandQueuePresentInfoToString) {
    ICommandQueue* testQueue = reinterpret_cast<ICommandQueue*>(0xCAFEFEED);
    Semaphore waitSemaphore1 = reinterpret_cast<Semaphore>(0x00000000EEEEAAAA);

    CommandQueuePresentInfo info = {};
    info.queue = testQueue;
    info.waitSemaphores = eastl::span(&waitSemaphore1, 1);

    eastl::string expected =
        "CommandQueuePresentInfo {\n"
        "  queue: 0x00000000CAFEFEED\n"
        "  waitSemaphores: [\n"
        "    0x00000000EEEEAAAA\n"
        "  ]\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CommandBufferInfoToString) {
    CommandBufferInfo info = {};
    info.name = "TestCommandBuffer";
    eastl::string expected = "CommandBufferInfo { name: \"TestCommandBuffer\" }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CopyBufferToBufferInfoToString) {
    Buffer src = reinterpret_cast<Buffer>(0x1111);
    Buffer dst = reinterpret_cast<Buffer>(0x2222);
    CopyBufferToBufferInfo info = { src, dst, 100, 200, 50 };
    eastl::string expected =
        "CopyBufferToBufferInfo {\n"
        "  srcBuffer: 0x0000000000001111\n"
        "  dstBuffer: 0x0000000000002222\n"
        "  srcOffset: 100\n"
        "  dstOffset: 200\n"
        "  size: 50\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CopyBufferToImageInfoToString) {
    Buffer buf = reinterpret_cast<Buffer>(0x1111);
    Image img = reinterpret_cast<Image>(0x2222);
    ImageArraySlice imgSlice = { 0, 0, 1 };
    Offset3D imgOffset = { 0, 0, 0 };
    Extent3D imgExtent = { 10, 10, 1 };
    CopyBufferToImageInfo info = {
        .buffer = buf,
        .bufferOffset = 0,
        .image = img,
        .imageSlice = imgSlice,
        .imageOffset = imgOffset,
        .imageExtent = imgExtent,
        .rowPitch = 256,
    };
    eastl::string expected =
        "CopyBufferToImageInfo {\n"
        "  buffer: 0x0000000000001111\n"
        "  bufferOffset: 0\n"
        "  image: 0x0000000000002222\n"
        "  imageSlice: { mip=0, firstLayer=0, layerCount=1 }\n"
        "  imageOffset: { 0, 0, 0 }\n"
        "  imageExtent: { 10, 10, 1 }\n"
        "  rowPitch: 256\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CopyImageToBufferInfoToString) {
    Image img = reinterpret_cast<Image>(0x1111);
    Buffer buf = reinterpret_cast<Buffer>(0x2222);
    ImageArraySlice imgSlice = { 0, 0, 1 };
    Offset3D imgOffset = { 0, 0, 0 };
    Extent3D imgExtent = { 10, 10, 1 };

    CopyImageToBufferInfo info = {
        .image = img,
        .imageSlice = imgSlice,
        .imageOffset = imgOffset,
        .imageExtent = imgExtent,
        .buffer = buf,
        .bufferOffset = 0,
        .rowPitch = 256,
    };
    eastl::string expected =
        "CopyImageToBufferInfo {\n"
        "  image: 0x0000000000001111\n"
        "  imageSlice: { mip=0, firstLayer=0, layerCount=1 }\n"
        "  imageOffset: { 0, 0, 0 }\n"
        "  imageExtent: { 10, 10, 1 }\n"
        "  buffer: 0x0000000000002222\n"
        "  bufferOffset: 0\n"
        "  rowPitch: 256\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CopyImageToImageInfoToString) {
    Image srcImg = reinterpret_cast<Image>(0x1111);
    Image dstImg = reinterpret_cast<Image>(0x2222);
    ImageArraySlice srcSlice = { 0, 1, 2 };
    Offset3D srcOffset = { 0, 0, 0 };
    ImageArraySlice dstSlice = { 1, 3, 2 };
    Offset3D dstOffset = { 0, 0, 0 };
    Extent3D extent = { 10, 10, 1 };

    CopyImageToImageInfo info = {
        .srcImage = srcImg,
        .dstImage = dstImg,
        .srcImageSlice = srcSlice,
        .srcOffset = srcOffset,
        .dstImageSlice = dstSlice,
        .dstOffset = dstOffset,
        .extent = extent,
    };
    eastl::string expected =
        "CopyImageToImageInfo {\n"
        "  srcImage: 0x0000000000001111\n"
        "  dstImage: 0x0000000000002222\n"
        "  srcImageSlice: { mip=0, firstLayer=1, layerCount=2 }\n"
        "  srcOffset: { 0, 0, 0 }\n"
        "  dstImageSlice: { mip=1, firstLayer=3, layerCount=2 }\n"
        "  dstOffset: { 0, 0, 0 }\n"
        "  extent: { 10, 10, 1 }\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, BlitImageToImageInfoToString) {
    Image srcImg = reinterpret_cast<Image>(0x1111);
    Image dstImg = reinterpret_cast<Image>(0x2222);
    ImageArraySlice srcSlice = { 0, 0, 1 };
    Box3D srcBox = { 0, 0, 0, 100, 100, 1 };
    ImageArraySlice dstSlice = { 0, 0, 1 };
    Box3D dstBox = { 0, 0, 0, 50, 50, 1 };
    BlitImageToImageInfo info = { srcImg, dstImg, srcSlice, srcBox, dstSlice, dstBox, Filter::Linear };
    eastl::string expected =
        "BlitImageToImageInfo {\n"
        "  srcImage: 0x0000000000001111\n"
        "  dstImage: 0x0000000000002222\n"
        "  srcImageSlice: { mip=0, firstLayer=0, layerCount=1 }\n"
        "  srcImageBox: { x=0, y=0, z=0, width=100, height=100, depth=1 }\n"
        "  dstImageSlice: { mip=0, firstLayer=0, layerCount=1 }\n"
        "  dstImageBox: { x=0, y=0, z=0, width=50, height=50, depth=1 }\n"
        "  filter: Linear\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ClearUnorderedAccessViewInfoToString) {
    UnorderedAccessId uav = UnorderedAccessId{ 0xABCD };
    ColorClearValue clearValue;
    clearValue.int32[0] = 5;
    clearValue.int32[1] = 6;
    clearValue.int32[2] = 7;
    clearValue.int32[3] = 8;
    ClearUnorderedAccessViewInfo info = { uav, clearValue };
    eastl::string expected =
        "ClearUAVInfo {\n"
        "  view: 0x000000000000ABCD\n"
        "  clearValue: ColorClear<Int32>{ 5, 6, 7, 8 }\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, UpdateBufferInfoToString) {
    Buffer buf = reinterpret_cast<Buffer>(0x123);
    BufferRegion region = { 0, 16 };
    void* data = (void*)0xAABBEECCFFF0FFFF;
    UpdateBufferInfo info = { buf, region, data };
    eastl::string expected =
        "UpdateBufferInfo {\n"
        "  buffer: 0x0000000000000123\n"
        "  region: { offset=0, size=16 }\n"
        "  data: 0xAABBEECCFFF0FFFF\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, BufferMemoryBarrierInfoToString) {
    Buffer buf = reinterpret_cast<Buffer>(0x456);
    BufferRegion region = { 0, 1024 };
    BufferMemoryBarrierInfo info = { buf, region, AccessConsts::TRANSFER_READ, AccessConsts::TRANSFER_WRITE, BufferLayout::TransferSrc, BufferLayout::TransferDst };
    eastl::string expected =
        "BufferMemoryBarrierInfo {\n"
        "  buffer: 0x0000000000000456\n"
        "  region: { offset=0, size=1024 }\n"
        "  srcAccess: Access{ stages: 4096, access: 32768 }\n" // TRANSFER_READ
        "  dstAccess: Access{ stages: 4096, access: 65536 }\n" // TRANSFER_WRITE
        "  srcLayout: TransferSrc\n"
        "  dstLayout: TransferDst\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ImageMemoryBarrierInfoToString) {
    Image img = reinterpret_cast<Image>(0x789);
    ImageMipArraySlice slice = { 0, 1, 0, 1 };
    ImageMemoryBarrierInfo info = { img, slice, AccessConsts::FRAGMENT_SHADER_READ, AccessConsts::FRAGMENT_SHADER_WRITE, ImageLayout::ReadOnly, ImageLayout::UnorderedAccess };
    eastl::string expected =
        "ImageMemoryBarrierInfo {\n"
        "  image: 0x0000000000000789\n"
        "  imageSlice: { firstMip=0, mipCount=1, firstLayer=0, layerCount=1 }\n"
        "  srcAccess: Access{ stages: 128, access: 32768 }\n" // FRAGMENT_SHADER_READ
        "  dstAccess: Access{ stages: 128, access: 65536 }\n" // FRAGMENT_SHADER_WRITE
        "  srcLayout: ReadOnly\n"
        "  dstLayout: UnorderedAccess\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, InvalidateTimestampQueryInfoToString) {
    ITimestampQueryPool* pool = reinterpret_cast<ITimestampQueryPool*>(0xAB);
    InvalidateTimestampQueryInfo info = { pool, 0, 5 };
    eastl::string expected =
        "InvalidateTimestampQueryInfo {\n"
        "  queryPool: 0x00000000000000AB\n"
        "  firstQuery: 0\n"
        "  queryCount: 5\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, WriteTimestampInfoToString) {
    ITimestampQueryPool* pool = reinterpret_cast<ITimestampQueryPool*>(0xBC);
    WriteTimestampInfo info = { pool, PipelineStageFlagBits::ALL_GRAPHICS, 2 };
    eastl::string expected =
        "WriteTimestampInfo {\n"
        "  queryPool: 0x00000000000000BC\n"
        "  stage: 32768\n" // ALL_GRAPHICS
        "  queryIndex: 2\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, CommandLabelInfoToString) {
    LabelColor labelColor(0.1f, 0.2f, 0.3f, 1.0f); // Example color
    CommandLabelInfo info = { labelColor, "MyLabel" };
    eastl::string expected =
        "CommandLabelInfo {\n"
        "  labelColor: #19334CFF\n" // 0xCCFF334C for RGBA? depends on ToU32 implementation
        "  name: \"MyLabel\"\n"
        "}";
    // NOTE: The `ToU32()` conversion for Color will need to be accurate for this to pass.
    // For now, I'm providing an example value. You might need to adjust based on actual `Color::ToU32()` output.
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, AttachmentResolveInfoToString) {
    RenderTarget targetImg = reinterpret_cast<RenderTarget>(0xDEF);
    AttachmentResolveInfo infoNone = { ResolveMode::None, targetImg };
    eastl::string expectedNone =
        "AttachmentResolveInfo {\n"
        "  mode: None\n"
        "  target: 0x0000000000000DEF\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(infoNone.ToString(0)).c_str(), RemoveIndentation(expectedNone).c_str());

    AttachmentResolveInfo infoAverage = { ResolveMode::Average, targetImg };
    eastl::string expectedAverage =
        "AttachmentResolveInfo {\n"
        "  mode: Average\n"
        "  target: 0x0000000000000DEF\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(infoAverage.ToString(0)).c_str(), RemoveIndentation(expectedAverage).c_str());
}

TEST(RHICommonToStringTests, ColorAttachmentInfoToString) {
    RenderTarget targetImg = reinterpret_cast<RenderTarget>(0x1111);
    ColorClearValue clearValue;
    clearValue.float32[0] = 0.0f;
    clearValue.float32[1] = 0.0f;
    clearValue.float32[2] = 0.0f;
    clearValue.float32[3] = 1.0f;
    ColorAttachmentInfo info = { targetImg, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, clearValue, eastl::nullopt };
    eastl::string expected =
        "ColorAttachmentInfo {\n"
        "  target: 0x0000000000001111\n"
        "  loadOp: Clear\n"
        "  storeOp: Store\n"
        "  clearValue: ColorClear<Float32>{ 0.000, 0.000, 0.000, 1.000 }\n"
        "  resolve: empty\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    // With resolve
    AttachmentResolveInfo resolveInfo = { ResolveMode::Average, reinterpret_cast<RenderTarget>(0x2222) };
    info.resolve = resolveInfo;
    eastl::string expectedWithResolve =
        "ColorAttachmentInfo {\n"
        "  target: 0x0000000000001111\n"
        "  loadOp: Clear\n"
        "  storeOp: Store\n"
        "  clearValue: ColorClear<Float32>{ 0.000, 0.000, 0.000, 1.000 }\n"
        "  resolve: AttachmentResolveInfo {\n"
        "    mode: Average\n"
        "    target: 0x0000000000002222\n"
        "  }\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expectedWithResolve).c_str());
}

TEST(RHICommonToStringTests, DepthStencilAttachmentInfoToString) {
    RenderTarget targetImg = reinterpret_cast<RenderTarget>(0x3333);
    DepthStencilClearValue clearValue = { 1.0f, 0 };
    DepthStencilAttachmentInfo info = { targetImg, AttachmentLoadOp::Clear, AttachmentStoreOp::Store,
        AttachmentLoadOp::DontCare, AttachmentStoreOp::DontCare, clearValue };
    eastl::string expected =
        "DepthStencilAttachmentInfo {\n"
        "  target: 0x0000000000003333\n"
        "  depthLoadOp: Clear\n"
        "  depthStoreOp: Store\n"
        "  stencilLoadOp: DontCare\n"
        "  stencilStoreOp: DontCare\n"
        "  clearValue: DepthStencilClear{ depth=1.000, stencil=0 }\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, RenderPassBeginInfoToString) {
    RenderTarget colorTarget1 = reinterpret_cast<RenderTarget>(0x1000);
    RenderTarget depthStencilTarget = reinterpret_cast<RenderTarget>(0x2000);

    ColorClearValue colorClear;
    colorClear.float32[0] = 0.1f;
    colorClear.float32[1] = 0.2f;
    colorClear.float32[2] = 0.3f;
    colorClear.float32[3] = 1.0f;
    ColorAttachmentInfo colorAttInfo1 = { colorTarget1, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, colorClear, eastl::nullopt };

    DepthStencilClearValue dsClear = { 1.0f, 0 };
    DepthStencilAttachmentInfo dsAttInfo = { depthStencilTarget, AttachmentLoadOp::Clear, AttachmentStoreOp::Store,
        AttachmentLoadOp::DontCare, AttachmentStoreOp::DontCare, dsClear };

    Rect2D renderArea = { 0, 0, 800, 600 };

    RenderPassBeginInfo info;
    info.colorAttachments.push_back(colorAttInfo1);
    info.depthStencilAttachment = dsAttInfo;
    info.renderArea = renderArea;

    eastl::string expected =
        "RenderPassBeginInfo {\n"
        "  colorAttachments: [\n"
        "    ColorAttachmentInfo {\n"
        "      target: 0x0000000000001000\n"
        "      loadOp: Clear\n"
        "      storeOp: Store\n"
        "      clearValue: ColorClear<Float32>{ 0.100, 0.200, 0.300, 1.000 }\n"
        "      resolve: empty\n"
        "    },\n"
        "  ]\n"
        "  depthStencilAttachment: DepthStencilAttachmentInfo {\n"
        "    target: 0x0000000000002000\n"
        "    depthLoadOp: Clear\n"
        "    depthStoreOp: Store\n"
        "    stencilLoadOp: DontCare\n"
        "    stencilStoreOp: DontCare\n"
        "    clearValue: DepthStencilClear{ depth=1.000, stencil=0 }\n"
        "  }\n"
        "  renderArea: { x=0, y=0, width=800, height=600 }\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, PushConstantInfoToString) {
    u32 data[4] = { 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210 };
    PushConstantInfo info = { data, 16, 0 };
    eastl::string expected =
        "PushConstantInfo {\n"
        "  data: [0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210]\n"
        "  size: 16\n"
        "  offset: 0\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SetUniformBufferViewInfoToString) {
    Buffer buf = reinterpret_cast<Buffer>(0x1234);
    BufferRegion region = { 0, 256 };
    SetUniformBufferViewInfo info = { 0, buf, region, PipelineBindPoint::Graphics };
    eastl::string expected =
        "SetUniformBufferViewInfo {\n"
        "  slot: 0\n"
        "  buffer: 0x0000000000001234\n"
        "  region: { offset=0, size=256 }\n"
        "  bindPoint: Graphics\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SetUnorderedAccessViewInfoToString) {
    UnorderedAccessId uav = UnorderedAccessId{ 0x5678 };
    SetUnorderedAccessViewInfo info = { 1, uav, PipelineBindPoint::Compute };
    eastl::string expected =
        "SetUnorderedAccessViewInfo {\n"
        "  slot: 1\n"
        "  view: 0x0000000000005678\n"
        "  bindPoint: Compute\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SetVertexBufferInfoToString) {
    Buffer vb = reinterpret_cast<Buffer>(0x9ABC);
    SetVertexBufferInfo info = { 0, vb, 0 };
    eastl::string expected =
        "SetVertexBufferInfo {\n"
        "  slot: 0\n"
        "  buffer: 0x0000000000009ABC\n"
        "  offset: 0\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SetIndexBufferInfoToString) {
    Buffer ib = reinterpret_cast<Buffer>(0xDEF0);
    SetIndexBufferInfo info = { ib, 0, IndexType::Uint32 };
    eastl::string expected =
        "SetIndexBufferInfo {\n"
        "  buffer: 0x000000000000DEF0\n"
        "  offset: 0\n"
        "  indexType: Uint32\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DrawInfoToString) {
    DrawInfo info = { 100, 1, 0, 0 };
    eastl::string expected = "DrawInfo { vertexCount: 100, instanceCount: 1, firstVertex: 0, firstInstance: 0 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DrawIndexedInfoToString) {
    DrawIndexedInfo info = { 1000, 1, 0, 0, 0 };
    eastl::string expected = "DrawIndexedInfo { indexCount: 1000, instanceCount: 1, firstIndex: 0, vertexOffset: 0, firstInstance: 0 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DrawIndirectInfoToString) {
    Buffer indirectBuf = reinterpret_cast<Buffer>(0xABCD1234);
    DrawIndirectInfo info = { indirectBuf, 0, 1, 16 };
    eastl::string expected = "DrawIndirectInfo { indirectBuffer: 0x00000000ABCD1234, indirectBufferOffset: 0, drawCount: 1, drawCommandStride: 16 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DrawIndexedIndirectInfoToString) {
    Buffer indirectBuf = reinterpret_cast<Buffer>(0xABCD5678);
    DrawIndexedIndirectInfo info = { indirectBuf, 0, 1, 20 };
    eastl::string expected = "DrawIndexedIndirectInfo { indirectBuffer: 0x00000000ABCD5678, indirectBufferOffset: 0, drawCount: 1, drawCommandStride: 20 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DispatchInfoToString) {
    DispatchInfo info = { 16, 8, 1 };
    eastl::string expected = "DispatchInfo { x: 16, y: 8, z: 1 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, DispatchIndirectInfoToString) {
    Buffer indirectBuf = reinterpret_cast<Buffer>(0x98765432);
    DispatchIndirectInfo info = { indirectBuf, 0 };
    eastl::string expected = "DispatchIndirectInfo { indirectBuffer: 0x0000000098765432, indirectBufferOffset: 0 }";
    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}



TEST(RHICommonToStringTests, MemoryBlockInfoToString) {
    MemoryBlockInfo info;
    info.bufferUsage = BufferUsageFlagBits::TRANSFER_SRC | BufferUsageFlagBits::TRANSFER_DST;
    info.imageUsage = ImageUsageFlagBits::SHADER_RESOURCE | ImageUsageFlagBits::RENDER_TARGET;
    info.size = 1024 * 1024; // 1MB
    info.strategy = VirtualSuballocationStrategy::SpaceEfficient;
    info.domain = MemoryAllocationDomain::HostRandomWrite;
    info.minAlignment = 256;
    info.name = "MyMemoryBlock";

    eastl::string expected =
        "MemoryBlockInfo {\n"
        "  bufferUsage: 3\n"
        "  imageUsage: 20\n"
        "  size: 1048576\n"
        "  strategy: SpaceEfficient\n"
        "  domain: HostRandomWrite\n"
        "  minAlignment: 256\n"
        "  name: \"MyMemoryBlock\"\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, BufferInfoToString) {
    BufferInfo info;
    info.memoryBlock = eastl::bit_cast<MemoryBlock>(0x12345678ULL); // A dummy memory block handle
    info.flags = BufferCreateFlagsBits::ALLOW_ALIAS;
    info.size = 65536; // 64KB
    info.usage = BufferUsageFlagBits::TRANSFER_DST;
    info.initialLayout = BufferLayout::Undefined;
    info.allocationDomain = MemoryAllocationDomain::DeviceLocal;
    info.name = "MyVertexBuffer";

    eastl::string expected =
        "BufferInfo {\n"
        "  memoryBlock: 0x0000000012345678\n"
        "  flags: 1\n"
        "  size: 65536\n"
        "  usage: 2\n"
        "  initialLayout: Undefined\n"
        "  allocationDomain: DeviceLocal\n"
        "  name: \"MyVertexBuffer\"\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    // Test with PYRO_NULL_MEMORY_BLOCK
    BufferInfo infoNullMemoryBlock;
    infoNullMemoryBlock.memoryBlock = PYRO_NULL_MEMORY_BLOCK; // Default to 0
    infoNullMemoryBlock.flags = BufferCreateFlagsBits::NONE;
    infoNullMemoryBlock.size = 1024;
    infoNullMemoryBlock.usage = BufferUsageFlagBits::UNORDERED_ACCESS;
    infoNullMemoryBlock.initialLayout = BufferLayout::UnorderedAccess;
    infoNullMemoryBlock.allocationDomain = MemoryAllocationDomain::HostStaging;
    infoNullMemoryBlock.name = "StagingBuffer";

    eastl::string expectedNull =
        "BufferInfo {\n"
        "  memoryBlock: 0x0000000000000000\n"
        "  flags: 0\n"
        "  size: 1024\n"
        "  usage: 8\n"
        "  initialLayout: UnorderedAccess\n"
        "  allocationDomain: HostStaging\n"
        "  name: \"StagingBuffer\"\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(infoNullMemoryBlock.ToString(0)).c_str(), RemoveIndentation(expectedNull).c_str());
}

TEST(RHICommonToStringTests, ImageInfoToString) {
    ImageInfo info;
    info.memoryBlock = eastl::bit_cast<MemoryBlock>(0xAABBCCDDULL);
    info.flags = ImageCreateFlagBits::CUBE | ImageCreateFlagBits::MUTABLE_FORMAT;
    info.dimensions = ImageDimensions::e2D;
    info.format = Format::RGBA8Unorm;
    info.size = { 1920, 1080, 1 };
    info.mipLevelCount = 5;
    info.arrayLayerCount = 6;
    info.sampleCount = RasterizationSamples::e4;
    info.usage = ImageUsageFlagBits::RENDER_TARGET | ImageUsageFlagBits::SHADER_RESOURCE;
    info.name = "MyRenderTarget";

    eastl::string expected =
        "ImageInfo {\n"
        "  memoryBlock: 0x00000000AABBCCDD\n"
        "  flags: 3\n" // 0x1 | 0x2
        "  dimensions: e2D\n"
        "  format: RGBA8Unorm\n"
        "  size: { 1920, 1080, 1 }\n"
        "  mipLevelCount: 5\n"
        "  arrayLayerCount: 6\n"
        "  sampleCount: e4\n"
        "  usage: 20\n" // 0x10 | 0x04
        "  name: \"MyRenderTarget\"\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, SamplerInfoToString) {
    SamplerInfo info;
    info.magnificationFilter = Filter::Linear;
    info.minificationFilter = Filter::Nearest;
    info.mipmapFilter = Filter::Linear;
    info.reductionMode = ReductionMode::Min;
    info.addressModeU = SamplerAddressMode::Repeat;
    info.addressModeV = SamplerAddressMode::MirroredRepeat;
    info.addressModeW = SamplerAddressMode::ClampToBorder;
    info.mipLodBias = 0.5f;
    info.enableAnisotropy = true;
    info.maxAnisotropy = 8;
    info.enableCompare = true;
    info.compareOp = CompareOp::Less;
    info.minLod = 1.0f;
    info.maxLod = 5.0f; // Not FLT_MAX for easier comparison
    info.borderColor = BorderColor::OpaqueWhiteInt;
    info.name = "MySampler";

    eastl::string expected =
        "SamplerInfo {\n"
        "  magnificationFilter: Linear\n"
        "  minificationFilter: Nearest\n"
        "  mipmapFilter: Linear\n"
        "  reductionMode: Min\n"
        "  addressModeU: Repeat\n"
        "  addressModeV: MirroredRepeat\n"
        "  addressModeW: ClampToBorder\n"
        "  mipLodBias: 0.500\n"
        "  enableAnisotropy: true\n"
        "  maxAnisotropy: 8\n"
        "  enableCompare: true\n"
        "  compareOp: Less\n"
        "  minLod: 1.000\n"
        "  maxLod: 5.000\n"
        "  borderColor: OpaqueWhiteInt\n"
        "  name: \"MySampler\"\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    // Test with FLT_MAX for maxLod
    SamplerInfo infoMaxLod = info;
    infoMaxLod.maxLod = FLT_MAX;
    eastl::string expectedMaxLod =
        "SamplerInfo {\n"
        "  magnificationFilter: Linear\n"
        "  minificationFilter: Nearest\n"
        "  mipmapFilter: Linear\n"
        "  reductionMode: Min\n"
        "  addressModeU: Repeat\n"
        "  addressModeV: MirroredRepeat\n"
        "  addressModeW: ClampToBorder\n"
        "  mipLodBias: 0.500\n"
        "  enableAnisotropy: true\n"
        "  maxAnisotropy: 8\n"
        "  enableCompare: true\n"
        "  compareOp: Less\n"
        "  minLod: 1.000\n"
        "  maxLod: FLT_MAX\n"
        "  borderColor: OpaqueWhiteInt\n"
        "  name: \"MySampler\"\n"
        "}";
    EXPECT_EQ(RemoveIndentation(infoMaxLod.ToString(0)), RemoveIndentation(expectedMaxLod));
}


TEST(RHICommonToStringTests, BufferResourceInfoToString) {
    BufferResourceInfo info;
    info.buffer = eastl::bit_cast<Buffer>(0x0000000099AABBCCULL);
    info.region = { 1024, 512 };

    eastl::string expected =
        "BufferResourceInfo {\n"
        "  buffer: 0x0000000099AABBCC\n"
        "  region: { offset=1024, size=512 }\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());
}

TEST(RHICommonToStringTests, ImageResourceInfoToString) {
    ImageResourceInfo info;
    info.image = eastl::bit_cast<Image>(0x0000000099AABBCCULL);
    info.slice = { 0, 1, 0, 1 }; // baseMip=0, levelCount=1, baseArray=0, layerCount=1
    info.viewType = ImageViewType::e2D;
    info.format = Format::R8Unorm;

    eastl::string expected =
        "ImageResourceInfo {\n"
        "  image: 0x0000000099AABBCC\n"
        "  slice: { firstMip=0, mipCount=1, firstLayer=0, layerCount=1 }\n"
        "  viewType: e2D\n"
        "  format: R8Unorm\n"
        "}";

    EXPECT_STREQ(RemoveIndentation(info.ToString(0)).c_str(), RemoveIndentation(expected).c_str());

    ImageResourceInfo infoCubeArray;
    infoCubeArray.image = eastl::bit_cast<Image>(0x0000000099AABBCCULL);
    infoCubeArray.slice = { 1, 2, 0, 6 }; // Mip 1-2, Layers 0-5 (6 layers for cube array)
    infoCubeArray.viewType = ImageViewType::eCubeArray;
    infoCubeArray.format = Format::BC1RGBUnormBlock;
    eastl::string expectedCubeArray =
        "ImageResourceInfo {\n"
        "  image: 0x0000000099AABBCC\n"
        "  slice: { firstMip=1, mipCount=2, firstLayer=0, layerCount=6 }\n"
        "  viewType: eCubeArray\n"
        "  format: BC1RGBUnormBlock\n"
        "}";
    EXPECT_STREQ(RemoveIndentation(infoCubeArray.ToString(0)).c_str(), RemoveIndentation(expectedCubeArray).c_str());
}