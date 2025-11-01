#include <RHIDX12/Core.hpp>
#include <gtest/gtest.h>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::RHIDX12;
using namespace PyroshockStudios::Types;

TEST(RHIDX12TypeConversions, ToDXGIFormat) {
    // Basic Formats
    EXPECT_EQ(ToDXGIFormat(Format::R8Unorm), DXGI_FORMAT_R8_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::RGBA8Unorm), DXGI_FORMAT_R8G8B8A8_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BGRA8Unorm), DXGI_FORMAT_B8G8R8A8_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::R16Sfloat), DXGI_FORMAT_R16_FLOAT);
    EXPECT_EQ(ToDXGIFormat(Format::RGBA16Sfloat), DXGI_FORMAT_R16G16B16A16_FLOAT);
    EXPECT_EQ(ToDXGIFormat(Format::R32Sfloat), DXGI_FORMAT_R32_FLOAT);
    EXPECT_EQ(ToDXGIFormat(Format::RGBA32Sfloat), DXGI_FORMAT_R32G32B32A32_FLOAT);

    // SRGB Formats
    EXPECT_EQ(ToDXGIFormat(Format::RGBA8Srgb), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    EXPECT_EQ(ToDXGIFormat(Format::BGRA8Srgb), DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

    // Packed Formats
    EXPECT_EQ(ToDXGIFormat(Format::BGRA4Unorm), DXGI_FORMAT_B4G4R4A4_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BGR565Unorm), DXGI_FORMAT_B5G6R5_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BGR5A1Unorm), DXGI_FORMAT_B5G5R5A1_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::A2RGB10Unorm), DXGI_FORMAT_R10G10B10A2_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::A2RGB10Uint), DXGI_FORMAT_R10G10B10A2_UINT);
    EXPECT_EQ(ToDXGIFormat(Format::E5BGR9Ufloat), DXGI_FORMAT_R9G9B9E5_SHAREDEXP);
    EXPECT_EQ(ToDXGIFormat(Format::B10GR11Ufloat), DXGI_FORMAT_R11G11B10_FLOAT);

    // Depth/Stencil Formats
    EXPECT_EQ(ToDXGIFormat(Format::D16Unorm), DXGI_FORMAT_D16_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::D24UnormS8Uint), DXGI_FORMAT_D24_UNORM_S8_UINT);
    EXPECT_EQ(ToDXGIFormat(Format::D32Sfloat), DXGI_FORMAT_D32_FLOAT);
    EXPECT_EQ(ToDXGIFormat(Format::D32SfloatS8Uint), DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
    // Note: D16UnormS8Uint maps to UNKNOWN  as there's no direct DXGI for D16S8
    EXPECT_EQ(ToDXGIFormat(Format::D16UnormS8Uint), DXGI_FORMAT_UNKNOWN);
    // Note: S8Uint maps to UNKNOWN as there's no direct DXGI for S8 only
    EXPECT_EQ(ToDXGIFormat(Format::S8Uint), DXGI_FORMAT_UNKNOWN);


    // Compressed Formats
    EXPECT_EQ(ToDXGIFormat(Format::BC1RGBUnormBlock), DXGI_FORMAT_BC1_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BC1RGBAUnormBlock), DXGI_FORMAT_BC1_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BC1RGBSrgbBlock), DXGI_FORMAT_BC1_UNORM_SRGB);
    EXPECT_EQ(ToDXGIFormat(Format::BC1RGBASrgbBlock), DXGI_FORMAT_BC1_UNORM_SRGB);
    EXPECT_EQ(ToDXGIFormat(Format::BC7UnormBlock), DXGI_FORMAT_BC7_UNORM);
    EXPECT_EQ(ToDXGIFormat(Format::BC7SrgbBlock), DXGI_FORMAT_BC7_UNORM_SRGB);

    // Unknown/Default
    EXPECT_EQ(ToDXGIFormat(Format::Undefined), DXGI_FORMAT_UNKNOWN);        // The switch default returns UNKNOWN
    EXPECT_EQ(ToDXGIFormat(Format::Inherit), DXGI_FORMAT_UNKNOWN);          // The switch default returns UNKNOWN
    EXPECT_EQ(ToDXGIFormat(static_cast<Format>(999)), DXGI_FORMAT_UNKNOWN); // Test an invalid enum value
}

TEST(RHIDX12TypeConversions, ToD3D12SamplerAddressMode) {
    EXPECT_EQ(ToD3D12SamplerAddressMode(SamplerAddressMode::Repeat), D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    EXPECT_EQ(ToD3D12SamplerAddressMode(SamplerAddressMode::MirroredRepeat), D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
    EXPECT_EQ(ToD3D12SamplerAddressMode(SamplerAddressMode::ClampToEdge), D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    EXPECT_EQ(ToD3D12SamplerAddressMode(SamplerAddressMode::ClampToBorder), D3D12_TEXTURE_ADDRESS_MODE_BORDER);
    EXPECT_EQ(ToD3D12SamplerAddressMode(SamplerAddressMode::MirrorClampToEdge), D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE);
    EXPECT_EQ(ToD3D12SamplerAddressMode(static_cast<SamplerAddressMode>(999)), D3D12_TEXTURE_ADDRESS_MODE_WRAP); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12BorderColor) {
    // Note: We are comparing pointers to static arrays.
    // The equality check will be true if they return the same static array.

    FLOAT transparentBlack[] = { 0.f, 0.f, 0.f, 0.f };
    FLOAT opaqueBlack[] = { 0.f, 0.f, 0.f, 1.f };
    FLOAT opaqueWhite[] = { 1.f, 1.f, 1.f, 1.f };

    // Test TransparentBlack
    FLOAT* resultTPB = ToD3D12BorderColor(BorderColor::TransparentBlackFloat);
    EXPECT_NE(resultTPB, nullptr); // Ensure it's not null
    EXPECT_EQ(resultTPB[0], transparentBlack[0]);
    EXPECT_EQ(resultTPB[1], transparentBlack[1]);
    EXPECT_EQ(resultTPB[2], transparentBlack[2]);
    EXPECT_EQ(resultTPB[3], transparentBlack[3]);

    resultTPB = ToD3D12BorderColor(BorderColor::TransparentBlackInt);
    EXPECT_NE(resultTPB, nullptr);
    EXPECT_EQ(resultTPB[0], transparentBlack[0]); // Check with float values.

    // Test OpaqueBlack
    FLOAT* resultOPB = ToD3D12BorderColor(BorderColor::OpaqueBlackFloat);
    EXPECT_NE(resultOPB, nullptr);
    EXPECT_EQ(resultOPB[0], opaqueBlack[0]);
    EXPECT_EQ(resultOPB[3], opaqueBlack[3]); // Check alpha

    resultOPB = ToD3D12BorderColor(BorderColor::OpaqueBlackInt);
    EXPECT_NE(resultOPB, nullptr);
    EXPECT_EQ(resultOPB[3], opaqueBlack[3]);

    // Test OpaqueWhite
    FLOAT* resultOPW = ToD3D12BorderColor(BorderColor::OpaqueWhiteFloat);
    EXPECT_NE(resultOPW, nullptr);
    EXPECT_EQ(resultOPW[0], opaqueWhite[0]);
    EXPECT_EQ(resultOPW[3], opaqueWhite[3]);

    resultOPW = ToD3D12BorderColor(BorderColor::OpaqueWhiteInt);
    EXPECT_NE(resultOPW, nullptr);
    EXPECT_EQ(resultOPW[3], opaqueWhite[3]);

    // Default case
    FLOAT* resultDefault = ToD3D12BorderColor(static_cast<BorderColor>(999));
    EXPECT_NE(resultDefault, nullptr);
    EXPECT_EQ(resultDefault[3], opaqueBlack[3]); // Should default to opaque black
}

TEST(RHIDX12TypeConversions, ToD3D12CompareOp) {
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::Never), D3D12_COMPARISON_FUNC_NEVER);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::Less), D3D12_COMPARISON_FUNC_LESS);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::Equal), D3D12_COMPARISON_FUNC_EQUAL);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::LessOrEqual), D3D12_COMPARISON_FUNC_LESS_EQUAL);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::Greater), D3D12_COMPARISON_FUNC_GREATER);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::NotEqual), D3D12_COMPARISON_FUNC_NOT_EQUAL);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::GreaterOrEqual), D3D12_COMPARISON_FUNC_GREATER_EQUAL);
    EXPECT_EQ(ToD3D12CompareOp(CompareOp::Always), D3D12_COMPARISON_FUNC_ALWAYS);
    EXPECT_EQ(ToD3D12CompareOp(static_cast<CompareOp>(999)), D3D12_COMPARISON_FUNC_ALWAYS); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12BlendFactor) {
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::Zero), D3D12_BLEND_ZERO);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::One), D3D12_BLEND_ONE);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::SrcColor), D3D12_BLEND_SRC_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusSrcColor), D3D12_BLEND_INV_SRC_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::DstColor), D3D12_BLEND_DEST_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusDstColor), D3D12_BLEND_INV_DEST_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::SrcAlpha), D3D12_BLEND_SRC_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusSrcAlpha), D3D12_BLEND_INV_SRC_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::DstAlpha), D3D12_BLEND_DEST_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusDstAlpha), D3D12_BLEND_INV_DEST_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::ConstantColor), D3D12_BLEND_BLEND_FACTOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusConstantColor), D3D12_BLEND_INV_BLEND_FACTOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::ConstantAlpha), D3D12_BLEND_BLEND_FACTOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusConstantAlpha), D3D12_BLEND_INV_BLEND_FACTOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::SrcAlphaSaturate), D3D12_BLEND_SRC_ALPHA_SAT);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::Src1Color), D3D12_BLEND_SRC1_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusSrc1Color), D3D12_BLEND_INV_SRC1_COLOR);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::Src1Alpha), D3D12_BLEND_SRC1_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(BlendFactor::OneMinusSrc1Alpha), D3D12_BLEND_INV_SRC1_ALPHA);
    EXPECT_EQ(ToD3D12BlendFactor(static_cast<BlendFactor>(999)), D3D12_BLEND_ONE); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12BlendOp) {
    EXPECT_EQ(ToD3D12BlendOp(BlendOp::Add), D3D12_BLEND_OP_ADD);
    EXPECT_EQ(ToD3D12BlendOp(BlendOp::Subtract), D3D12_BLEND_OP_SUBTRACT);
    EXPECT_EQ(ToD3D12BlendOp(BlendOp::ReverseSubtract), D3D12_BLEND_OP_REV_SUBTRACT);
    EXPECT_EQ(ToD3D12BlendOp(BlendOp::Min), D3D12_BLEND_OP_MIN);
    EXPECT_EQ(ToD3D12BlendOp(BlendOp::Max), D3D12_BLEND_OP_MAX);
    EXPECT_EQ(ToD3D12BlendOp(static_cast<BlendOp>(999)), D3D12_BLEND_OP_ADD); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12PrimitiveTopology) {
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::PointList, 0), D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::LineList, 0), D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::LineStrip, 0), D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::TriangleList, 0), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::TriangleStrip, 0), D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::TriangleFan, 0), D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::LineListWithAdjacency, 0), D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::LineStripWithAdjacency, 0), D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::TriangleListWithAdjacency, 0), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::TriangleStripWithAdjacency, 0), D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);

    // Patch List requires numControlPoints
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::PatchList, 1), D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::PatchList, 3), D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    EXPECT_EQ(ToD3D12PrimitiveTopology(PrimitiveTopology::PatchList, 32), D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST);

    EXPECT_EQ(ToD3D12PrimitiveTopology(static_cast<PrimitiveTopology>(999), 0), D3D_PRIMITIVE_TOPOLOGY_UNDEFINED); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12PrimitiveTopologyType) {
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::PointList), D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::LineList), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::LineStrip), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::LineListWithAdjacency), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::TriangleList), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::TriangleStrip), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::TriangleFan), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(PrimitiveTopology::PatchList), D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
    EXPECT_EQ(ToD3D12PrimitiveTopologyType(static_cast<PrimitiveTopology>(999)), D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12CullMode) {
    EXPECT_EQ(ToD3D12CullMode(FaceCull::None), D3D12_CULL_MODE_NONE);
    EXPECT_EQ(ToD3D12CullMode(FaceCull::Front), D3D12_CULL_MODE_FRONT);
    EXPECT_EQ(ToD3D12CullMode(FaceCull::Back), D3D12_CULL_MODE_BACK);
    EXPECT_EQ(ToD3D12CullMode(static_cast<FaceCull>(999)), D3D12_CULL_MODE_NONE); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12StencilOp) {
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::Keep), D3D12_STENCIL_OP_KEEP);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::Zero), D3D12_STENCIL_OP_ZERO);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::Replace), D3D12_STENCIL_OP_REPLACE);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::IncrementClamp), D3D12_STENCIL_OP_INCR_SAT);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::DecrementClamp), D3D12_STENCIL_OP_DECR_SAT);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::Invert), D3D12_STENCIL_OP_INVERT);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::IncrementWrap), D3D12_STENCIL_OP_INCR);
    EXPECT_EQ(ToD3D12StencilOp(StencilOp::DecrementWrap), D3D12_STENCIL_OP_DECR);
    EXPECT_EQ(ToD3D12StencilOp(static_cast<StencilOp>(999)), D3D12_STENCIL_OP_KEEP); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12BufferResourceState) {
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::Undefined), D3D12_RESOURCE_STATE_COMMON);
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::UnorderedAccess), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::ReadOnly), D3D12_RESOURCE_STATE_GENERIC_READ);
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::TransferSrc), D3D12_RESOURCE_STATE_COPY_SOURCE);
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::TransferDst), D3D12_RESOURCE_STATE_COPY_DEST);
    EXPECT_EQ(ToD3D12BufferResourceState(BufferLayout::Identity), D3D12_RESOURCE_STATE_COMMON);         // Identity maps to COMMON
    EXPECT_EQ(ToD3D12BufferResourceState(static_cast<BufferLayout>(999)), D3D12_RESOURCE_STATE_COMMON); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12ImageResourceState) {
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::Undefined), D3D12_RESOURCE_STATE_COMMON);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::UnorderedAccess), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::ReadOnly), D3D12_RESOURCE_STATE_GENERIC_READ);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::RenderTarget), D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_DEPTH_WRITE);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::RenderTargetReadOnly), D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_DEPTH_READ);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::TransferSrc), D3D12_RESOURCE_STATE_COPY_SOURCE);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::TransferDst), D3D12_RESOURCE_STATE_COPY_DEST);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::BlitSrc), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::BlitDst), D3D12_RESOURCE_STATE_RENDER_TARGET);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::PresentSrc), D3D12_RESOURCE_STATE_PRESENT);
    EXPECT_EQ(ToD3D12ImageResourceState(ImageLayout::Identity), D3D12_RESOURCE_STATE_COMMON);         // Identity maps to COMMON
    EXPECT_EQ(ToD3D12ImageResourceState(static_cast<ImageLayout>(999)), D3D12_RESOURCE_STATE_COMMON); // Default case
}

TEST(RHIDX12TypeConversions, ToD3D12Rect) {
    Rect2D r1 = { 0, 0, 100, 50 };
    D3D12_RECT expected1 = { 0, 0, 100, 50 };
    D3D12_RECT actual1 = ToD3D12Rect(r1);
    EXPECT_EQ(actual1.left, expected1.left);
    EXPECT_EQ(actual1.top, expected1.top);
    EXPECT_EQ(actual1.right, expected1.right);
    EXPECT_EQ(actual1.bottom, expected1.bottom);

    Rect2D r2 = { 10, 20, 200, 150 };
    D3D12_RECT expected2 = { 10, 20, 210, 170 }; // x + width, y + height
    D3D12_RECT actual2 = ToD3D12Rect(r2);
    EXPECT_EQ(actual2.left, expected2.left);
    EXPECT_EQ(actual2.top, expected2.top);
    EXPECT_EQ(actual2.right, expected2.right);
    EXPECT_EQ(actual2.bottom, expected2.bottom);
}

TEST(RHIDX12TypeConversions, ToD3D12Box) {
    Box3D b1 = { 0, 0, 0, 10, 20, 30 };
    D3D12_BOX expected1 = { 0, 0, 0, 10, 20, 30 };
    D3D12_BOX actual1 = ToD3D12Box(b1);
    EXPECT_EQ(actual1.left, expected1.left);
    EXPECT_EQ(actual1.top, expected1.top);
    EXPECT_EQ(actual1.front, expected1.front);
    EXPECT_EQ(actual1.right, expected1.right);
    EXPECT_EQ(actual1.bottom, expected1.bottom);
    EXPECT_EQ(actual1.back, expected1.back);

    Box3D b2 = { 5, 10, 15, 100, 50, 25 };
    D3D12_BOX expected2 = { 5, 10, 15, 105, 60, 40 }; // x + width, y + height, z + depth
    D3D12_BOX actual2 = ToD3D12Box(b2);
    EXPECT_EQ(actual2.left, expected2.left);
    EXPECT_EQ(actual2.top, expected2.top);
    EXPECT_EQ(actual2.front, expected2.front);
    EXPECT_EQ(actual2.right, expected2.right);
    EXPECT_EQ(actual2.bottom, expected2.bottom);
    EXPECT_EQ(actual2.back, expected2.back);
}