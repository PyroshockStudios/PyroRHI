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

#include <EASTL/algorithm.h>
#include <EASTL/array.h>
#include <EASTL/string.h>
#include <EASTL/tuple.h>
#include <EASTL/variant.h>
#include <EASTL/vector.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Core.hpp>
#include <compare>

namespace PyroshockStudios {
    inline namespace RHI {
        using DeviceSize = u64;
        using DeviceAddress = DeviceSize;

        enum struct Format : i32 {
            Inherit = -1,
            Undefined = 0,
            BGRA4Unorm = 3,
            BGR565Unorm = 5,
            BGR5A1Unorm = 7,
            R8Unorm = 9,
            R8Snorm = 10,
            R8Uint = 13,
            R8Sint = 14,
            R8Srgb = 15,
            RG8Unorm = 16,
            RG8Snorm = 17,
            RG8Uint = 20,
            RG8Sint = 21,
            RG8Srgb = 22,
            RGB8Unorm = 23,
            RGB8Snorm = 24,
            RGB8Uint = 27,
            RGB8Sint = 28,
            RGB8Srgb = 29,
            BGR8Unorm = 30,
            BGR8Snorm = 31,
            BGR8Uint = 34,
            BGR8Sint = 35,
            BGR8Srgb = 36,
            RGBA8Unorm = 37,
            RGBA8Snorm = 38,
            RGBA8Uint = 41,
            RGBA8Sint = 42,
            RGBA8Srgb = 43,
            BGRA8Unorm = 44,
            BGRA8Snorm = 45,
            BGRA8Uint = 48,
            BGRA8Sint = 49,
            BGRA8Srgb = 50,
            A2RGB10Unorm = 58,
            A2RGB10Snorm = 59,
            A2RGB10Uint = 62,
            A2RGB10Sint = 63,
            R16Unorm = 70,
            R16Snorm = 71,
            R16Uint = 74,
            R16Sint = 75,
            R16Sfloat = 76,
            RG16Unorm = 77,
            RG16Snorm = 78,
            RG16Uint = 81,
            RG16Sint = 82,
            RG16Sfloat = 83,
            RGB16Unorm = 84,
            RGB16Snorm = 85,
            RGB16Uint = 88,
            RGB16Sint = 89,
            RGB16Sfloat = 90,
            RGBA16Unorm = 91,
            RGBA16Snorm = 92,
            RGBA16Uint = 95,
            RGBA16Sint = 96,
            RGBA16Sfloat = 97,
            R32Uint = 98,
            R32Sint = 99,
            R32Sfloat = 100,
            RG32Uint = 101,
            RG32Sint = 102,
            RG32Sfloat = 103,
            RGB32Uint = 104,
            RGB32Sint = 105,
            RGB32Sfloat = 106,
            RGBA32Uint = 107,
            RGBA32Sint = 108,
            RGBA32Sfloat = 109,
            B10GR11Ufloat = 122,
            E5BGR9Ufloat = 123,
            D16Unorm = 124,
            X8D24Unorm = 125,
            D32Sfloat = 126,
            S8Uint = 127,
            D16UnormS8Uint = 128,
            D24UnormS8Uint = 129,
            D32SfloatS8Uint = 130,
            BC1RGBUnormBlock = 131,
            BC1RGBSrgbBlock = 132,
            BC1RGBAUnormBlock = 133,
            BC1RGBASrgbBlock = 134,
            BC2UnormBlock = 135,
            BC2SrgbBlock = 136,
            BC3UnormBlock = 137,
            BC3SrgbBlock = 138,
            BC4UnormBlock = 139,
            BC4SnormBlock = 140,
            BC5UnormBlock = 141,
            BC5SnormBlock = 142,
            BC6HUfloatBlock = 143,
            BC6HSfloatBlock = 144,
            BC7UnormBlock = 145,
            BC7SrgbBlock = 146,
        };
        struct FormatFeatureFlagsProperties {
            using Data = u32;
        };
        using FormatFeatureFlags = Flags<FormatFeatureFlagsProperties>;
        struct FormatFeatureBits {
            static constexpr inline FormatFeatureFlags SHADER_RESOURCE = { 0x00000001 };
            static constexpr inline FormatFeatureFlags UNORDERED_ACCESS = { 0x00000002 };
            static constexpr inline FormatFeatureFlags UNORDERED_ACCESS_ATOMIC = { 0x00000004 };
            // TODO: implement [RW]TextureBuffer in the future
            // static constexpr inline FormatFeatureFlags UNIFORM_TEXEL_BUFFER = { 0x00000008 };
            // static constexpr inline FormatFeatureFlags STORAGE_TEXEL_BUFFER = { 0x00000010 };
            // static constexpr inline FormatFeatureFlags STORAGE_TEXEL_BUFFER_ATOMIC = { 0x00000020 };
            static constexpr inline FormatFeatureFlags VERTEX_BUFFER = { 0x00000040 };
            static constexpr inline FormatFeatureFlags COLOR_TARGET = { 0x00000080 };
            static constexpr inline FormatFeatureFlags COLOR_TARGET_BLEND = { 0x00000100 };
            static constexpr inline FormatFeatureFlags DEPTH_STENCIL_TARGET = { 0x00000200 };
            static constexpr inline FormatFeatureFlags BLIT_SRC = { 0x00000400 };
            static constexpr inline FormatFeatureFlags BLIT_DST = { 0x00000800 };
            static constexpr inline FormatFeatureFlags LINEAR_FILTER = { 0x00001000 };
            static constexpr inline FormatFeatureFlags TRANSFER_SRC = { 0x00004000 };
            static constexpr inline FormatFeatureFlags TRANSFER_DST = { 0x00008000 };
        };

        enum struct SamplerAddressMode : i32 {
            Repeat = 0,
            MirroredRepeat = 1,
            ClampToEdge = 2,
            ClampToBorder = 3,
            MirrorClampToEdge = 4
        };

        enum struct BorderColor : i32 {
            TransparentBlackFloat = 0,
            TransparentBlackInt = 1,
            OpaqueBlackFloat = 2,
            OpaqueBlackInt = 3,
            OpaqueWhiteFloat = 4,
            OpaqueWhiteInt = 5,
        };

        enum struct CompareOp : i32 {
            Never = 0,
            Less = 1,
            Equal = 2,
            LessOrEqual = 3,
            Greater = 4,
            NotEqual = 5,
            GreaterOrEqual = 6,
            Always = 7,
        };

        enum struct BlendFactor : i32 {
            Zero = 0,
            One = 1,
            SrcColor = 2,
            OneMinusSrcColor = 3,
            DstColor = 4,
            OneMinusDstColor = 5,
            SrcAlpha = 6,
            OneMinusSrcAlpha = 7,
            DstAlpha = 8,
            OneMinusDstAlpha = 9,
            ConstantColor = 10,
            OneMinusConstantColor = 11,
            ConstantAlpha = 12,
            OneMinusConstantAlpha = 13,
            SrcAlphaSaturate = 14,
            Src1Color = 15,
            OneMinusSrc1Color = 16,
            Src1Alpha = 17,
            OneMinusSrc1Alpha = 18
        };

        enum struct BlendOp : i32 {
            Add = 0,
            Subtract = 1,
            ReverseSubtract = 2,
            Min = 3,
            Max = 4,
        };

        enum struct TesselationDomainOrigin : i32 {
            LowerLeft = 0,
            UpperLeft = 1,
        };

        enum struct PrimitiveTopology : i32 {
            PointList = 0,
            LineList = 1,
            LineStrip = 2,
            TriangleList = 3,
            TriangleStrip = 4,
            TriangleFan = 5,
            LineListWithAdjacency = 6,
            LineStripWithAdjacency = 7,
            TriangleListWithAdjacency = 8,
            TriangleStripWithAdjacency = 9,
            PatchList = 10
        };

        enum struct PolygonMode : i32 {
            Triangle = 0,
            Line = 1,
            Point = 2,
        };

        enum struct LineMode : i32 {
            Normal = 0,
            Smooth = 1
            // TODO: add more line modes: stippled, etc.
        };

        enum struct WindingOrder : i32 {
            CounterClockwise = 0,
            Clockwise = 1,
        };

        enum struct FaceCull : i32 {
            None = 0,
            Front = 1,
            Back = 2
        };

        enum struct StencilOp : i32 {
            Keep = 0,
            Zero = 1,
            Replace = 2,
            IncrementClamp = 3,
            DecrementClamp = 4,
            Invert = 5,
            IncrementWrap = 6,
            DecrementWrap = 7
        };

        enum struct RasterizationSamples : u32 {
            e1 = 1,
            e2 = 2,
            e4 = 4,
            e8 = 8,
            e16 = 16,
            e32 = 32,
            e64 = 64
        };

        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples operator|(RasterizationSamples lhs, RasterizationSamples rhs) {
            return static_cast<RasterizationSamples>(
                static_cast<u32>(lhs) | static_cast<u32>(rhs));
        }
        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples operator&(RasterizationSamples lhs, RasterizationSamples rhs) {
            return static_cast<RasterizationSamples>(
                static_cast<u32>(lhs) & static_cast<u32>(rhs));
        }
        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples operator^(RasterizationSamples lhs, RasterizationSamples rhs) {
            return static_cast<RasterizationSamples>(
                static_cast<u32>(lhs) ^ static_cast<u32>(rhs));
        }
        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples& operator|=(RasterizationSamples& lhs, RasterizationSamples rhs) {
            lhs = lhs | rhs;
            return lhs;
        }
        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples& operator&=(RasterizationSamples& lhs, RasterizationSamples rhs) {
            lhs = lhs & rhs;
            return lhs;
        }
        PYRO_NODISCARD PYRO_FORCEINLINE RasterizationSamples& operator^=(RasterizationSamples& lhs, RasterizationSamples rhs) {
            lhs = lhs ^ rhs;
            return lhs;
        }

        struct ColorComponentFlagsProperties {
            using Data = u32;
        };

        using ColorComponentFlags = Flags<ColorComponentFlagsProperties>;
        struct ColorComponentFlagBits {
            static inline constexpr ColorComponentFlags NONE = { 0x00000000 };
            static inline constexpr ColorComponentFlags R = { 0x00000001 };
            static inline constexpr ColorComponentFlags G = { 0x00000002 };
            static inline constexpr ColorComponentFlags B = { 0x00000004 };
            static inline constexpr ColorComponentFlags A = { 0x00000008 };
            static inline constexpr ColorComponentFlags ALL = { R | G | B | A };
        };

        struct AccessTypeFlagsProperties {
            using Data = u64;
        };

        using AccessTypeFlags = Flags<AccessTypeFlagsProperties>;
        struct AccessTypeFlagBits {
            static inline constexpr AccessTypeFlags NONE = { 0x00000000 };
            static inline constexpr AccessTypeFlags READ = { 0x00008000 };
            static inline constexpr AccessTypeFlags WRITE = { 0x00010000 };
            static inline constexpr AccessTypeFlags READ_WRITE = READ | WRITE;
        };

        struct PipelineStageFlagsProperties {
            using Data = u64;
        };

        using PipelineStageFlags = Flags<PipelineStageFlagsProperties>;
        struct PipelineStageFlagBits {
            static inline constexpr PipelineStageFlags NONE = { 0x00000000ull };

            static inline constexpr PipelineStageFlags TOP_OF_PIPE = { 0x00000001ull };
            static inline constexpr PipelineStageFlags DRAW_INDIRECT = { 0x00000002ull };
            static inline constexpr PipelineStageFlags VERTEX_INPUT = { 0x00000004ULL };
            static inline constexpr PipelineStageFlags VERTEX_SHADER = { 0x00000008ull };
            static inline constexpr PipelineStageFlags HULL_SHADER = { 0x00000010ull };
            static inline constexpr PipelineStageFlags DOMAIN_SHADER = { 0x00000020ull };
            static inline constexpr PipelineStageFlags GEOMETRY_SHADER = { 0x00000040ull };
            static inline constexpr PipelineStageFlags FRAGMENT_SHADER = { 0x00000080ull };
            static inline constexpr PipelineStageFlags EARLY_FRAGMENT_TESTS = { 0x00000100ull };
            static inline constexpr PipelineStageFlags LATE_FRAGMENT_TESTS = { 0x00000200ull };
            static inline constexpr PipelineStageFlags COLOR_ATTACHMENT_OUTPUT = { 0x00000400ull };
            static inline constexpr PipelineStageFlags COMPUTE_SHADER = { 0x00000800ull };
            static inline constexpr PipelineStageFlags TRANSFER = { 0x00001000ull };
            static inline constexpr PipelineStageFlags BOTTOM_OF_PIPE = { 0x00002000ull };
            static inline constexpr PipelineStageFlags HOST = { 0x00004000ull };
            static inline constexpr PipelineStageFlags ALL_GRAPHICS = { 0x00008000ull };
            static inline constexpr PipelineStageFlags ALL_COMMANDS = { 0x00010000ull };
            static inline constexpr PipelineStageFlags COPY = { 0x100000000ull };
            static inline constexpr PipelineStageFlags RESOLVE = { 0x200000000ull };
            static inline constexpr PipelineStageFlags BLIT = { 0x400000000ull };
            static inline constexpr PipelineStageFlags CLEAR = { 0x800000000ull };
            static inline constexpr PipelineStageFlags INDEX_INPUT = { 0x1000000000ull };
            static inline constexpr PipelineStageFlags PRE_RASTERIZATION_SHADERS = { 0x4000000000ull };
            static inline constexpr PipelineStageFlags TASK_SHADER = { 0x00080000ull };
            static inline constexpr PipelineStageFlags MESH_SHADER = { 0x00100000ull };
            static inline constexpr PipelineStageFlags ACCELERATION_STRUCTURE_BUILD = { 0x02000000ull };
            static inline constexpr PipelineStageFlags RAY_TRACING_SHADER = { 0x00200000ull };
        };

        struct Access {
            PipelineStageFlags stages = PipelineStageFlagBits::NONE;
            AccessTypeFlags type = AccessTypeFlagBits::NONE;

            PYRO_FORCEINLINE constexpr Access& operator|=(const Access& other) noexcept {
                stages |= other.stages;
                type |= other.type;
                return *this;
            }

            PYRO_FORCEINLINE constexpr Access& operator&=(const Access& other) noexcept {
                stages &= other.stages;
                type &= other.type;
                return *this;
            }

            PYRO_FORCEINLINE constexpr Access& operator^=(const Access& other) noexcept {
                stages ^= other.stages;
                type ^= other.type;
                return *this;
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr Access operator~() const noexcept {
                return { .stages = ~stages, .type = ~type };
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr Access operator|(const Access& other) const noexcept {
                return { .stages = stages | other.stages, .type = type | other.type };
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr Access operator&(const Access& other) const noexcept {
                return { .stages = stages & other.stages, .type = type & other.type };
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr Access operator^(const Access& other) const noexcept {
                return { .stages = stages ^ other.stages, .type = type ^ other.type };
            }
            PYRO_NODISCARD PYRO_FORCEINLINE friend auto operator<=>(Access const&, Access const&) = default;

            PYRO_FORCEINLINE constexpr operator bool() const {
                return stages && type;
            }

            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        namespace AccessConsts {
            static inline constexpr Access NONE = { .stages = PipelineStageFlagBits::NONE, .type = AccessTypeFlagBits::NONE };

            static inline constexpr Access TOP_OF_PIPE_READ = { .stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access DRAW_INDIRECT_READ = { .stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access VERTEX_INPUT_READ = { .stages = PipelineStageFlagBits::VERTEX_INPUT, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access VERTEX_SHADER_READ = { .stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access HULL_SHADER_READ = { .stages = PipelineStageFlagBits::HULL_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access DOMAIN_SHADER_READ = { .stages = PipelineStageFlagBits::DOMAIN_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access GEOMETRY_SHADER_READ = { .stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access FRAGMENT_SHADER_READ = { .stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access EARLY_FRAGMENT_TESTS_READ = { .stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access LATE_FRAGMENT_TESTS_READ = { .stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_READ = { .stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access COMPUTE_SHADER_READ = { .stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access TRANSFER_READ = { .stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access BOTTOM_OF_PIPE_READ = { .stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access HOST_READ = { .stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access ALL_GRAPHICS_READ = { .stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access READ = { .stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access COPY_READ = { .stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access RESOLVE_READ = { .stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access BLIT_READ = { .stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access CLEAR_READ = { .stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access INDEX_INPUT_READ = { .stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access PRE_RASTERIZATION_SHADERS_READ = { .stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access TASK_SHADER_READ = { .stages = PipelineStageFlagBits::TASK_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access MESH_SHADER_READ = { .stages = PipelineStageFlagBits::MESH_SHADER, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access ACCELERATION_STRUCTURE_BUILD_READ = { .stages = PipelineStageFlagBits::ACCELERATION_STRUCTURE_BUILD, .type = AccessTypeFlagBits::READ };
            static inline constexpr Access RAY_TRACING_SHADER_READ = { .stages = PipelineStageFlagBits::RAY_TRACING_SHADER, .type = AccessTypeFlagBits::READ };

            static inline constexpr Access TOP_OF_PIPE_WRITE = { .stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access DRAW_INDIRECT_WRITE = { .stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access VERTEX_INPUT_WRITE = { .stages = PipelineStageFlagBits::VERTEX_INPUT, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access VERTEX_SHADER_WRITE = { .stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access HULL_SHADER_WRITE = { .stages = PipelineStageFlagBits::HULL_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access DOMAIN_SHADER_WRITE = { .stages = PipelineStageFlagBits::DOMAIN_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access GEOMETRY_SHADER_WRITE = { .stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access FRAGMENT_SHADER_WRITE = { .stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access EARLY_FRAGMENT_TESTS_WRITE = { .stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access LATE_FRAGMENT_TESTS_WRITE = { .stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_WRITE = { .stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access COMPUTE_SHADER_WRITE = { .stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access TRANSFER_WRITE = { .stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access BOTTOM_OF_PIPE_WRITE = { .stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access HOST_WRITE = { .stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access ALL_GRAPHICS_WRITE = { .stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access WRITE = { .stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access COPY_WRITE = { .stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access RESOLVE_WRITE = { .stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access BLIT_WRITE = { .stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access CLEAR_WRITE = { .stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access INDEX_INPUT_WRITE = { .stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access PRE_RASTERIZATION_SHADERS_WRITE = { .stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access TASK_SHADER_WRITE = { .stages = PipelineStageFlagBits::TASK_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access MESH_SHADER_WRITE = { .stages = PipelineStageFlagBits::MESH_SHADER, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access ACCELERATION_STRUCTURE_BUILD_WRITE = { .stages = PipelineStageFlagBits::ACCELERATION_STRUCTURE_BUILD, .type = AccessTypeFlagBits::WRITE };
            static inline constexpr Access RAY_TRACING_SHADER_WRITE = { .stages = PipelineStageFlagBits::RAY_TRACING_SHADER, .type = AccessTypeFlagBits::WRITE };

            static inline constexpr Access TOP_OF_PIPE_READ_WRITE = { .stages = PipelineStageFlagBits::TOP_OF_PIPE, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access DRAW_INDIRECT_READ_WRITE = { .stages = PipelineStageFlagBits::DRAW_INDIRECT, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access VERTEX_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::VERTEX_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access HULL_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::HULL_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access DOMAIN_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::DOMAIN_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access GEOMETRY_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::GEOMETRY_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access FRAGMENT_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::FRAGMENT_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access EARLY_FRAGMENT_TESTS_READ_WRITE = { .stages = PipelineStageFlagBits::EARLY_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access LATE_FRAGMENT_TESTS_READ_WRITE = { .stages = PipelineStageFlagBits::LATE_FRAGMENT_TESTS, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access COLOR_ATTACHMENT_OUTPUT_READ_WRITE = { .stages = PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access COMPUTE_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::COMPUTE_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access TRANSFER_READ_WRITE = { .stages = PipelineStageFlagBits::TRANSFER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access BOTTOM_OF_PIPE_READ_WRITE = { .stages = PipelineStageFlagBits::BOTTOM_OF_PIPE, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access HOST_READ_WRITE = { .stages = PipelineStageFlagBits::HOST, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access ALL_GRAPHICS_READ_WRITE = { .stages = PipelineStageFlagBits::ALL_GRAPHICS, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access READ_WRITE = { .stages = PipelineStageFlagBits::ALL_COMMANDS, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access COPY_READ_WRITE = { .stages = PipelineStageFlagBits::COPY, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access RESOLVE_READ_WRITE = { .stages = PipelineStageFlagBits::RESOLVE, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access BLIT_READ_WRITE = { .stages = PipelineStageFlagBits::BLIT, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access CLEAR_READ_WRITE = { .stages = PipelineStageFlagBits::CLEAR, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access INDEX_INPUT_READ_WRITE = { .stages = PipelineStageFlagBits::INDEX_INPUT, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access PRE_RASTERIZATION_SHADERS_READ_WRITE = { .stages = PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access TASK_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::TASK_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access MESH_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::MESH_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access ACCELERATION_STRUCTURE_BUILD_READ_WRITE = { .stages = PipelineStageFlagBits::ACCELERATION_STRUCTURE_BUILD, .type = AccessTypeFlagBits::READ_WRITE };
            static inline constexpr Access RAY_TRACING_SHADER_READ_WRITE = { .stages = PipelineStageFlagBits::RAY_TRACING_SHADER, .type = AccessTypeFlagBits::READ_WRITE };
        } // namespace AccessConsts

        enum struct PipelineBindPoint : i32 {
            None = -1,
            Graphics = 0,
            Compute = 1,
            RayTracing = 2
        };

        struct CommandQueueFlagsProperties {
            using Data = u32;
        };
        using CommandQueueFlags = Flags<CommandQueueFlagsProperties>;
        struct CommandQueueFlagBits {
            static constexpr inline CommandQueueFlags NONE = { 0x00000000 };
            static constexpr inline CommandQueueFlags GRAPHICS = { 0x00000001 };
            static constexpr inline CommandQueueFlags COMPUTE = { 0x00000002 };
            static constexpr inline CommandQueueFlags TRANSFER = { 0x00000004 };
        };

        struct ImageUsageFlagsProperties {
            using Data = u32;
        };

        using ImageUsageFlags = Flags<ImageUsageFlagsProperties>;
        struct ImageUsageFlagBits {
            static inline constexpr ImageUsageFlags NONE = { 0x00000000 };
            static inline constexpr ImageUsageFlags TRANSFER_SRC = { 0x00000001 };
            static inline constexpr ImageUsageFlags TRANSFER_DST = { 0x00000002 };
            static inline constexpr ImageUsageFlags SHADER_RESOURCE = { 0x00000004 };
            static inline constexpr ImageUsageFlags UNORDERED_ACCESS = { 0x00000008 };
            static inline constexpr ImageUsageFlags RENDER_TARGET = { 0x00000010 };
            static inline constexpr ImageUsageFlags TRANSIENT_ATTACHMENT = { 0x00000040 };
            static inline constexpr ImageUsageFlags FRAGMENT_DENSITY_MAP = { 0x00000200 };
            static inline constexpr ImageUsageFlags FRAGMENT_SHADING_RATE_ATTACHMENT = { 0x00000100 };
            static inline constexpr ImageUsageFlags HOST_TRANSFER = { 0x00400000 };

            static inline constexpr ImageUsageFlags BLIT_SRC = { 0x02000000 };
            static inline constexpr ImageUsageFlags BLIT_DST = { 0x04000000 };
        };


        struct BufferUsageFlagsProperties {
            using Data = u32;
        };

        using BufferUsageFlags = Flags<BufferUsageFlagsProperties>;
        struct BufferUsageFlagBits {
            static inline constexpr BufferUsageFlags NONE = { 0x00000000 };
            static inline constexpr BufferUsageFlags TRANSFER_SRC = { 0x00000001 };
            static inline constexpr BufferUsageFlags TRANSFER_DST = { 0x00000002 };
            static inline constexpr BufferUsageFlags SHADER_RESOURCE = { 0x00000004 };
            static inline constexpr BufferUsageFlags UNORDERED_ACCESS = { 0x00000008 };
            static inline constexpr BufferUsageFlags UNIFORM_BUFFER = { 0x00000004 };
            static inline constexpr BufferUsageFlags VERTEX_BUFFER = { 0x00000001 };
            static inline constexpr BufferUsageFlags INDEX_BUFFER = { 0x00000002 };
            static inline constexpr BufferUsageFlags DRAW_INDIRECT = { 0x00000010 };
            static inline constexpr BufferUsageFlags BUFFER_DEVICE_ADDRESS = { 0x00000020 };
            static inline constexpr BufferUsageFlags BYTE_ADDRESS_BUFFER = { 0x08000000 };
            static inline constexpr BufferUsageFlags ACCELERATION_STRUCTURE = { 0x10000000 };
            static inline constexpr BufferUsageFlags BLAS_GEOMETRY_BUFFER = { 0x20000000 };
            static inline constexpr BufferUsageFlags BLAS_INSTANCE_BUFFER = { 0x40000000 };
            static inline constexpr BufferUsageFlags ACCELERATION_STRUCTURE_SCRATCH_BUFFER = { 0x80000000 };
        };

        enum struct MemoryAllocationDomain : i32 {
            DeviceLocal = 0,
            HostStaging = 1,
            HostRandomWrite = 2,
            HostReadback = 3
        };

        // TODO, is this relevant?
        enum struct ColorSpace : i32 {
            SrgbNonlinear = 0,
            DisplayP3Nonlinear = 1000104001,
            ExtendedSrgbLinear = 1000104002,
            DisplayP3Linear = 1000104003,
            DciP3Nonlinear = 1000104004,
            Bt709Linear = 1000104005,
            Bt709Nonlinear = 1000104006,
            Bt2020Linear = 1000104007,
            Hdr10St2084 = 1000104008,
            Dolbyvision = 1000104009,
            Hdr10Hlg = 1000104010,
            AdobergbLinear = 1000104011,
            AdobergbNonlinear = 1000104012,
            PassThrough = 1000104013,
            ExtendedSrgbNonlinear = 1000104014,
            DisplayNativeAmd = 1000213000,
        };

        enum struct ImageLayout : i32 {
            Identity = -1,
            // FIXME: require UNDEFINED to be treated the same as other layouts
            Undefined = 0,
            UnorderedAccess = 1,
            ReadOnly = 2,
            RenderTarget = 3,
            RenderTargetReadOnly = 4,
            TransferSrc = 5,
            TransferDst = 6,
            BlitSrc = 7,
            BlitDst = 8,
            PresentSrc = 9,
        };

        enum struct BufferLayout : i32 {
            Identity = -1,
            // FIXME: require UNDEFINED to be treated the same as other layouts
            Undefined = 0,
            UnorderedAccess = 1,
            ReadOnly = 2,
            TransferSrc = 3,
            TransferDst = 4,
        };

        enum struct Filter : i32 {
            Nearest = 0,
            Linear = 1,
        };

        // TODO: check support on dx12
        enum struct ReductionMode : i32 {
            WeightedAverage = 0,
            Min = 1,
            Max = 2,
        };

        enum struct IndexType : i32 {
            Uint16 = 0,
            Uint32 = 1,
            Uint8 = 1000265000,
            None = 1000165000,
        };

        enum struct AttachmentLoadOp : i32 {
            Load = 0,
            Clear = 1,
            DontCare = 2,
        };
        enum struct AttachmentStoreOp : i32 {
            Store = 0,
            DontCare = 1,
        };

        struct DepthStencilClearValue {
            f32 depth = {};
            u32 stencil = {};

            PYRO_NODISCARD bool operator==(const DepthStencilClearValue&) const = default;
            PYRO_NODISCARD bool operator!=(const DepthStencilClearValue&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        union ColorClearValue {
            eastl::array<f32, 4> float32 = {};
            eastl::array<i32, 4> int32;
            eastl::array<u32, 4> uint32;

            PYRO_NODISCARD bool operator==(const ColorClearValue& other) const {
                return uint32 == other.uint32;
            }
            PYRO_NODISCARD bool operator!=(const ColorClearValue& other) const {
                return !(*this == other);
            }
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Offset2D {
            i32 x = {};
            i32 y = {};

            PYRO_NODISCARD friend auto operator<=>(const Offset2D&, const Offset2D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Offset3D {
            i32 x = {};
            i32 y = {};
            i32 z = {};

            PYRO_NODISCARD friend auto operator<=>(const Offset3D&, const Offset3D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Extent2D {
            u32 width = 1;
            u32 height = 1;

            PYRO_NODISCARD friend auto operator<=>(const Extent2D&, const Extent2D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Extent3D {
            u32 width = 1;
            u32 height = 1;
            u32 depth = 1;

            PYRO_NODISCARD friend auto operator<=>(const Extent3D&, const Extent3D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct ViewportInfo {
            f32 x = {};
            f32 y = {};
            f32 width = 1;
            f32 height = 1;
            f32 minDepth = 0.0f;
            f32 maxDepth = 1.0f;

            PYRO_NODISCARD friend auto operator<=>(const ViewportInfo&, const ViewportInfo&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Rect2D {
            i32 x = {};
            i32 y = {};
            i32 width = 1;
            i32 height = 1;

            PYRO_NODISCARD PYRO_FORCEINLINE static constexpr Rect2D Cut(Extent2D extent, Offset2D offset = {}) {
                return {
                    .x = offset.x,
                    .y = offset.y,
                    .width = static_cast<i32>(extent.width),
                    .height = static_cast<i32>(extent.height),
                };
            }

            PYRO_NODISCARD friend auto operator<=>(const Rect2D&, const Rect2D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct Box3D {
            i32 x = {};
            i32 y = {};
            i32 z = {};
            i32 width = 1;
            i32 height = 1;
            i32 depth = 1;

            PYRO_NODISCARD PYRO_FORCEINLINE static constexpr Box3D Cut(Extent3D extent, Offset3D offset = {}) {
                return {
                    .x = offset.x,
                    .y = offset.y,
                    .z = offset.z,
                    .width = static_cast<i32>(extent.width),
                    .height = static_cast<i32>(extent.height),
                    .depth = static_cast<i32>(extent.depth),
                };
            }

            PYRO_NODISCARD friend auto operator<=>(const Box3D&, const Box3D&) = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };
        static constexpr inline DeviceSize PYRO_MAX_DEVICE_SIZE = ~(static_cast<DeviceSize>(0));
        struct BufferRegion {
            DeviceSize offset = 0;
            DeviceSize size = PYRO_MAX_DEVICE_SIZE;

            PYRO_NODISCARD friend auto operator<=>(const BufferRegion&, const BufferRegion&) = default;

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr bool Contains(const BufferRegion& region) const noexcept {
                return offset <= region.offset && (offset + size) >= (region.offset + region.size);
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr bool Intersects(const BufferRegion& region) const noexcept {
                return !(region.offset >= offset + size || region.offset + region.size <= offset);
            }

            PYRO_NODISCARD constexpr BufferRegion Intersect(const BufferRegion& region) const noexcept {
                if (!Intersects(region))
                    return { 0, 0 };
                size_t start = eastl::max(offset, region.offset);
                size_t end = eastl::min(offset + size, region.offset + region.size);
                return { start, end - start };
            }

            PYRO_NODISCARD eastl::tuple<eastl::array<BufferRegion, 2>, size_t> Subtract(const BufferRegion& slice) const noexcept {
                eastl::array<BufferRegion, 2> result{};
                size_t count = 0;

                if (!Intersects(slice)) {
                    result[count++] = *this;
                    return { result, count };
                }

                size_t start = offset;
                size_t end = offset + size;
                size_t sliceStart = slice.offset;
                size_t sliceEnd = slice.offset + slice.size;

                // Left part (before slice)
                if (sliceStart > start) {
                    size_t leftSize = sliceStart - start;
                    if (leftSize > 0)
                        result[count++] = { start, leftSize };
                }

                // Right part (after slice)
                if (sliceEnd < end) {
                    size_t rightSize = end - sliceEnd;
                    if (rightSize > 0)
                        result[count++] = { sliceEnd, rightSize };
                }

                return { result, count };
            }
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        static constexpr inline u32 PYRO_REMAINING_MIP_LEVELS = ~(0U);
        static constexpr inline u32 PYRO_REMAINING_ARRAY_LAYERS = ~(0U);

        struct ImageMipArraySlice;
        struct ImageArraySlice;
        struct ImageSlice;

        struct ImageMipArraySlice {
            u32 baseMipLevel = 0;
            u32 levelCount = PYRO_REMAINING_MIP_LEVELS;
            u32 baseArrayLayer = 0;
            u32 layerCount = PYRO_REMAINING_ARRAY_LAYERS;

            PYRO_NODISCARD friend auto operator<=>(const ImageMipArraySlice&, const ImageMipArraySlice&) = default;

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr bool Contains(const ImageMipArraySlice& slice) const noexcept {
                return baseMipLevel <= slice.baseMipLevel && (baseMipLevel + levelCount) >= (slice.baseMipLevel + slice.levelCount) && baseArrayLayer <= slice.baseArrayLayer && (baseArrayLayer + layerCount) >= (slice.baseArrayLayer + slice.layerCount);
            }

            PYRO_NODISCARD PYRO_FORCEINLINE constexpr bool Intersects(const ImageMipArraySlice& slice) const noexcept {
                bool mipOverlap = !(slice.baseMipLevel >= baseMipLevel + levelCount || slice.baseMipLevel + slice.levelCount <= baseMipLevel);
                bool layerOverlap = !(slice.baseArrayLayer >= baseArrayLayer + layerCount || slice.baseArrayLayer + slice.layerCount <= baseArrayLayer);
                return mipOverlap && layerOverlap;
            }

            PYRO_NODISCARD constexpr ImageMipArraySlice Intersect(const ImageMipArraySlice& slice) const noexcept {
                if (!Intersects(slice))
                    return { 0, 0, 0, 0 };
                u32 startMip = eastl::max(baseMipLevel, slice.baseMipLevel);
                u32 endMip = eastl::min(baseMipLevel + levelCount, slice.baseMipLevel + slice.levelCount);
                u32 startLayer = eastl::max(baseArrayLayer, slice.baseArrayLayer);
                u32 endLayer = eastl::min(baseArrayLayer + layerCount, slice.baseArrayLayer + slice.layerCount);
                return { startMip, endMip - startMip, startLayer, endLayer - startLayer };
            }

            PYRO_NODISCARD eastl::tuple<eastl::array<ImageMipArraySlice, 4>, size_t> Subtract(const ImageMipArraySlice& slice) const noexcept {
                eastl::array<ImageMipArraySlice, 4> result{};
                size_t count = 0;

                if (!Intersects(slice)) {
                    result[count++] = *this;
                    return { result, count };
                }

                ImageMipArraySlice inter = Intersect(slice);

                // Top slice (mips before intersection)
                if (inter.baseMipLevel > baseMipLevel) {
                    u32 lCount = inter.baseMipLevel - baseMipLevel;
                    if (lCount > 0)
                        result[count++] = { baseMipLevel, lCount, baseArrayLayer, layerCount };
                }

                // Bottom slice (mips after intersection)
                u32 bottomMipStart = inter.baseMipLevel + inter.levelCount;
                u32 totalMips = baseMipLevel + levelCount;
                if (bottomMipStart < totalMips) {
                    u32 lCount = totalMips - bottomMipStart;
                    if (lCount > 0)
                        result[count++] = { bottomMipStart, lCount, baseArrayLayer, layerCount };
                }

                // Left slice (layers before intersection)
                if (inter.baseArrayLayer > baseArrayLayer) {
                    u32 lCount = inter.baseArrayLayer - baseArrayLayer;
                    if (lCount > 0)
                        result[count++] = { inter.baseMipLevel, inter.levelCount, baseArrayLayer, lCount };
                }

                // Right slice (layers after intersection)
                u32 rightLayerStart = inter.baseArrayLayer + inter.layerCount;
                u32 totalLayers = baseArrayLayer + layerCount;
                if (rightLayerStart < totalLayers) {
                    u32 lCount = totalLayers - rightLayerStart;
                    if (lCount > 0)
                        result[count++] = { inter.baseMipLevel, inter.levelCount, rightLayerStart, lCount };
                }

                return { result, count };
            }

            PYRO_NODISCARD PYRO_FORCEINLINE ImageArraySlice Slice(u32 mipLevel) const noexcept;
            PYRO_NODISCARD PYRO_FORCEINLINE ImageSlice Slice(u32 mipLevel, u32 arrayLevel) const noexcept;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct ImageArraySlice {
            u32 mipLevel = 0;
            u32 baseArrayLayer = 0;
            u32 layerCount = PYRO_REMAINING_ARRAY_LAYERS;

            PYRO_NODISCARD friend auto operator<=>(const ImageArraySlice&, const ImageArraySlice&) = default;

            PYRO_NODISCARD bool ContainedIn(const ImageMipArraySlice& slice) const noexcept {
                return slice.baseMipLevel <= mipLevel && mipLevel < slice.baseMipLevel + slice.levelCount && slice.baseArrayLayer <= baseArrayLayer && (baseArrayLayer + layerCount) <= (slice.baseArrayLayer + slice.layerCount);
            }

            PYRO_NODISCARD PYRO_FORCEINLINE ImageSlice Slice(u32 arrayLevel = 0) const noexcept;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct ImageSlice {
            u32 mipLevel = 0;
            u32 arrayLayer = 0;

            PYRO_NODISCARD friend auto operator<=>(const ImageSlice&, const ImageSlice&) = default;

            PYRO_NODISCARD bool ContainedIn(const ImageArraySlice& slice) const noexcept {
                return slice.mipLevel == mipLevel && slice.baseArrayLayer <= arrayLayer && arrayLayer < slice.baseArrayLayer + slice.layerCount;
            }

            PYRO_NODISCARD bool ContainedIn(const ImageMipArraySlice& slice) const noexcept {
                return slice.baseMipLevel <= mipLevel && mipLevel < slice.baseMipLevel + slice.levelCount && slice.baseArrayLayer <= arrayLayer && arrayLayer < slice.baseArrayLayer + slice.layerCount;
            }
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        PYRO_FORCEINLINE ImageArraySlice ImageMipArraySlice::Slice(u32 mipLevel) const noexcept {
            return { baseMipLevel + mipLevel, baseArrayLayer, layerCount };
        }
        PYRO_FORCEINLINE ImageSlice ImageMipArraySlice::Slice(u32 mipLevel, u32 arrayLevel) const noexcept {
            return { baseMipLevel + mipLevel, baseArrayLayer + arrayLevel };
        }
        PYRO_FORCEINLINE ImageSlice ImageArraySlice::Slice(u32 arrayLevel) const noexcept {
            return { mipLevel, baseArrayLayer + arrayLevel };
        }

#define PYRO_IMAGE_SLICE_RESOLVE_LAYERS(subSlice, fullLayerCount) \
    ((subSlice.layerCount == PYRO_REMAINING_ARRAY_LAYERS) ? (fullLayerCount - subSlice.baseArrayLayer) : subSlice.layerCount)
#define PYRO_IMAGE_SLICE_RESOLVE_LEVELS(subSlice, fullLevelCount) \
    ((subSlice.levelCount == PYRO_REMAINING_MIP_LEVELS) ? (fullLevelCount - subSlice.baseMipLevel) : subSlice.levelCount)

        struct DrawArgumentBuffer {
            u32 vertexCount = {};
            u32 instanceCount = {};
            u32 firstVertex = {};
            u32 firstInstance = {};
        };
        struct DrawIndexedArgumentBuffer {
            u32 indexCount = {};
            u32 instanceCount = {};
            u32 firstIndex = {};
            i32 vertexOffset = {};
            u32 firstInstance = {};
        };

        struct DispatchArgumentBuffer {
            u32 x = {};
            u32 y = {};
            u32 z = {};
        };

        struct LabelColor {
            static LabelColor RED;
            static LabelColor ORANGE;
            static LabelColor YELLOW;
            static LabelColor GREEN;
            static LabelColor CYAN;
            static LabelColor BLUE;
            static LabelColor VIOLET;
            static LabelColor MAGENTA;
            static LabelColor FADED_RED;
            static LabelColor FADED_ORANGE;
            static LabelColor FADED_YELLOW;
            static LabelColor FADED_GREEN;
            static LabelColor FADED_BLUE;
            static LabelColor FADED_VIOLET;
            static LabelColor FADED_MAGENTA;
            static LabelColor PINK;
            static LabelColor TURQUOISE;
            static LabelColor TEAL;
            static LabelColor WHITE;
            static LabelColor BLACK;

            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;
            float a = 1.0f;

            PYRO_NODISCARD friend auto operator<=>(const LabelColor&, const LabelColor&) = default;

            PYRO_NODISCARD PYRO_FORCEINLINE u32 ToU32() const {
                u8 ir = static_cast<u8>(r * 255);
                u8 ig = static_cast<u8>(g * 255);
                u8 ib = static_cast<u8>(b * 255);
                u8 ia = static_cast<u8>(a * 255);
                return static_cast<u32>(ir) << 24 |
                       static_cast<u32>(ig) << 16 |
                       static_cast<u32>(ib) << 8 |
                       static_cast<u32>(ia) << 0;
            }
        };

        inline LabelColor LabelColor::RED = { 1.0f, 0.0f, 0.0f, 1.0f };
        inline LabelColor LabelColor::ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f };
        inline LabelColor LabelColor::YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
        inline LabelColor LabelColor::GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
        inline LabelColor LabelColor::CYAN = { 0.0f, 1.0f, 1.0f, 1.0f };
        inline LabelColor LabelColor::BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
        inline LabelColor LabelColor::VIOLET = { 0.5f, 0.0f, 1.0f, 1.0f };
        inline LabelColor LabelColor::MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };

        inline LabelColor LabelColor::FADED_RED = { 1.0f, 0.5f, 0.5f, 1.0f };
        inline LabelColor LabelColor::FADED_ORANGE = { 1.0f, 0.75f, 0.5f, 1.0f };
        inline LabelColor LabelColor::FADED_YELLOW = { 1.0f, 1.0f, 0.5f, 1.0f };
        inline LabelColor LabelColor::FADED_GREEN = { 0.5f, 1.0f, 0.5f, 1.0f };
        inline LabelColor LabelColor::FADED_BLUE = { 0.5f, 0.5f, 1.0f, 1.0f };
        inline LabelColor LabelColor::FADED_VIOLET = { 0.75f, 0.5f, 1.0f, 1.0f };
        inline LabelColor LabelColor::FADED_MAGENTA = { 1.0f, 0.5f, 1.0f, 1.0f };

        inline LabelColor LabelColor::PINK = { 1.0f, 0.75f, 0.8f, 1.0f };
        inline LabelColor LabelColor::TURQUOISE = { 0.25f, 0.88f, 0.82f, 1.0f };
        inline LabelColor LabelColor::TEAL = { 0.0f, 0.5f, 0.5f, 1.0f };

        inline LabelColor LabelColor::WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline LabelColor LabelColor::BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };

        struct Transform {
            float matrix[3][4];

            // Default constructor (initializes to zero)
            Transform() {
                memset(matrix, 0, sizeof(matrix));
            }

            // Constructor for brace-initialization
            Transform(float m00, float m01, float m02, float m03,
                float m10, float m11, float m12, float m13,
                float m20, float m21, float m22, float m23) {
                matrix[0][0] = m00;
                matrix[0][1] = m01;
                matrix[0][2] = m02;
                matrix[0][3] = m03;
                matrix[1][0] = m10;
                matrix[1][1] = m11;
                matrix[1][2] = m12;
                matrix[1][3] = m13;
                matrix[2][0] = m20;
                matrix[2][1] = m21;
                matrix[2][2] = m22;
                matrix[2][3] = m23;
            }

            // Static identity matrix
            static const Transform IDENTITY;

            // Static zero matrix
            static const Transform ZERO;
        };

        // This represents an affine identity transformation
        // [ 1 0 0 0 ]
        // [ 0 1 0 0 ]
        // [ 0 0 1 0 ]
        inline const Transform Transform::IDENTITY = {
            1.0f, 0.0f, 0.0f, 0.0f, // Row 0
            0.0f, 1.0f, 0.0f, 0.0f, // Row 1
            0.0f, 0.0f, 1.0f, 0.0f  // Row 2
        };

        inline const Transform Transform::ZERO = {};
    } // namespace RHI
} // namespace PyroshockStudios