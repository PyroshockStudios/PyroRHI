#include <RHIDX12/Core.hpp>
#include <gtest/gtest.h>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::RHIDX12;
using namespace PyroshockStudios::Types;

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_R8) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R8Unorm), DXGI_FORMAT_R8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R8Snorm), DXGI_FORMAT_R8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R8Uint), DXGI_FORMAT_R8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R8Sint), DXGI_FORMAT_R8_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_RG8) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RG8Unorm), DXGI_FORMAT_R8G8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RG8Snorm), DXGI_FORMAT_R8G8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RG8Uint), DXGI_FORMAT_R8G8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RG8Sint), DXGI_FORMAT_R8G8_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_RGBA8) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA8Unorm), DXGI_FORMAT_R8G8B8A8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA8Snorm), DXGI_FORMAT_R8G8B8A8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA8Uint), DXGI_FORMAT_R8G8B8A8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA8Sint), DXGI_FORMAT_R8G8B8A8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA8Srgb), DXGI_FORMAT_R8G8B8A8_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_BGRA8) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BGRA8Unorm), DXGI_FORMAT_B8G8R8A8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BGRA8Srgb), DXGI_FORMAT_B8G8R8A8_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_R16) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R16Unorm), DXGI_FORMAT_R16_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R16Sfloat), DXGI_FORMAT_R16_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_RGBA16) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA16Unorm), DXGI_FORMAT_R16G16B16A16_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA16Sfloat), DXGI_FORMAT_R16G16B16A16_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_R32) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R32Uint), DXGI_FORMAT_R32_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::R32Sfloat), DXGI_FORMAT_R32_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_RGBA32) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA32Uint), DXGI_FORMAT_R32G32B32A32_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::RGBA32Sfloat), DXGI_FORMAT_R32G32B32A32_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_DepthStencil) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::D24UnormS8Uint), DXGI_FORMAT_R24G8_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::D32SfloatS8Uint), DXGI_FORMAT_R32G8X24_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::D16Unorm), DXGI_FORMAT_R16_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::D32Sfloat), DXGI_FORMAT_R32_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_BC) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC1RGBUnormBlock), DXGI_FORMAT_BC1_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC1RGBSrgbBlock), DXGI_FORMAT_BC1_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC3UnormBlock), DXGI_FORMAT_BC3_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC5UnormBlock), DXGI_FORMAT_BC5_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC6HUfloatBlock), DXGI_FORMAT_BC6H_TYPELESS);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::BC7UnormBlock), DXGI_FORMAT_BC7_TYPELESS);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormatTypeless_UnknownDefault) {
    EXPECT_EQ(ToDXGIFormatTypeless(Format::Undefined), DXGI_FORMAT_UNKNOWN);
    EXPECT_EQ(ToDXGIFormatTypeless(Format::Inherit), DXGI_FORMAT_UNKNOWN);
    EXPECT_EQ(ToDXGIFormatTypeless(static_cast<Format>(999)), DXGI_FORMAT_UNKNOWN);
}

TEST(RHIDX12TypeConversionsTypeless, ToDXGIFormat_IndexType) {
    EXPECT_EQ(ToDXGIFormat(IndexType::Uint16), DXGI_FORMAT_R16_UINT);
    EXPECT_EQ(ToDXGIFormat(IndexType::Uint32), DXGI_FORMAT_R32_UINT);
    EXPECT_EQ(ToDXGIFormat(IndexType::Uint8), DXGI_FORMAT_R8_UINT);
    EXPECT_EQ(ToDXGIFormat(IndexType::None), DXGI_FORMAT_UNKNOWN);
    EXPECT_EQ(ToDXGIFormat(static_cast<IndexType>(999)), DXGI_FORMAT_UNKNOWN);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_None) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::NONE);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_AllowUpdate) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::ALLOW_UPDATE);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_AllowCompaction) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::ALLOW_COMPACTION);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_PreferFastTrace) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::PREFER_FAST_TRACE);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_PreferFastBuild) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::PREFER_FAST_BUILD);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_LowMemory) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(AccelerationStructureCreateFlagBits::LOW_MEMORY);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASBuildFlags_Combined) {
    auto combined = AccelerationStructureCreateFlagBits::ALLOW_UPDATE | AccelerationStructureCreateFlagBits::PREFER_FAST_TRACE;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = ToD3D12ASBuildFlags(combined);
    EXPECT_EQ(flags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASGeometryFlags_None) {
    D3D12_RAYTRACING_GEOMETRY_FLAGS flags = ToD3D12ASGeometryFlags(AccelerationStructureGeometryFlagBits::NONE);
    EXPECT_EQ(flags, D3D12_RAYTRACING_GEOMETRY_FLAG_NONE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASGeometryFlags_Opaque) {
    D3D12_RAYTRACING_GEOMETRY_FLAGS flags = ToD3D12ASGeometryFlags(AccelerationStructureGeometryFlagBits::OPAQUE);
    EXPECT_EQ(flags, D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASGeometryFlags_NoDuplicateAnyHit) {
    D3D12_RAYTRACING_GEOMETRY_FLAGS flags = ToD3D12ASGeometryFlags(AccelerationStructureGeometryFlagBits::NO_DUPLICATE_ANY_HIT_INVOCATION);
    EXPECT_EQ(flags, D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION);
}

TEST(RHIDX12TypeConversionsTypeless, ToD3D12ASGeometryFlags_Combined) {
    auto combined = AccelerationStructureGeometryFlagBits::OPAQUE | AccelerationStructureGeometryFlagBits::NO_DUPLICATE_ANY_HIT_INVOCATION;
    D3D12_RAYTRACING_GEOMETRY_FLAGS flags = ToD3D12ASGeometryFlags(combined);
    EXPECT_EQ(flags, D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE | D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION);
}
