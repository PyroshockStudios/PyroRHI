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

#include <EASTL/fixed_vector.h>
#include <EASTL/optional.h>
#include <EASTL/span.h>
#include <EASTL/string.h>
#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Limits.hpp>
#include <PyroRHI/Api/Types.hpp>
#include <PyroRHI/Core.hpp>
#include <PyroRHI/Shader/Forward.hpp>
#include <PyroRHI/Shader/ShaderProgram.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        struct IDevice;

        /**
         * @brief Information for a single specialization constant in a shader.
         */
        struct SpecializationConstantInfo {
            /**
             * @brief Location index of the specialization constant within the shader.
             */
            u32 location = {};

            /**
             * @brief Value for the specialization constant. Supports multiple data types.
             */
            Union<b32, i32, u32, f32> data = {};

            PYRO_NODISCARD  bool operator==(const SpecializationConstantInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const SpecializationConstantInfo&) const = default;
        };

        /**
         * @brief Parameters for creating a shader stage with optional specialization constants.
         */
        struct ShaderInfo {
            /**
             * @brief Byte span to the compiled program.
             */
            eastl::span<const u8> program = {};

            /**
             * @brief List of specialization constants to set at pipeline creation time.
             */
            eastl::span<const SpecializationConstantInfo> specializationConstants = {};

            PYRO_NODISCARD  bool operator==(const ShaderInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const ShaderInfo&) const = default;
        };

        /**
         * @brief Parameters for creating a compute pipeline.
         */
        struct ComputePipelineInfo {
            /**
             * @brief Optional debug name for the pipeline.
             */
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const ComputePipelineInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const ComputePipelineInfo&) const = default;
        };

        /**
         * @brief Handle representing a compute pipeline object.
         */
        RHI_TYPED_PTR_HANDLE(ComputePipeline);

        /**
         * @brief Parameters describing a single vertex attribute in a vertex buffer.
         */
        struct VertexAttributeInfo {
            /**
             * @brief Location index in the shader input.
             */
            u32 location = 0;

            /**
             * @brief Binding index of the vertex buffer supplying this attribute.
             */
            u32 binding = 0;

            /**
             * @brief Data format of the vertex attribute. *MUST* not be `Undefined`
             */
            Format format = Format::Undefined;

            /**
             * @brief Byte offset within the vertex buffer binding.
             */
            u32 offset = 0;

            PYRO_NODISCARD  bool operator==(const VertexAttributeInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const VertexAttributeInfo&) const = default;
        };

        /**
         * @brief Parameters describing a vertex buffer binding.
         */
        struct VertexBindingInfo {
            /**
             * @brief Binding index of the vertex buffer.
             */
            u32 binding = 0;

            /**
             * @brief Stride (in bytes) between consecutive elements in the vertex buffer.
             */
            u32 stride = 0;

            /**
             * @brief Whether data is provided per-instance rather than per-vertex.
             */
            bool bPerInstance = false;

            PYRO_NODISCARD  bool operator==(const VertexBindingInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const VertexBindingInfo&) const = default;
        };

        /**
         * @brief Enum for dictating the participation in the depth/stencil tests.
         */
        enum struct DepthStencilTestState : i32 {
            /**
             * @brief No participation in depth/stencil test
             */
            Ignore = 0,
            /**
             * @brief Only reads depth/stencil input for test operations.
             */
            Read = 1,
            /**
             * @brief Performs depth-stencil tests, and enables depth/stencil writes to the render target.
             */
            ReadWrite = 2
        };

        /**
         * @brief Parameters for configuring depth testing behavior.
         */
        struct DepthTestInfo {
            /**
             * @brief Depth test compare operation.
             */
            CompareOp compareOp = CompareOp::LessOrEqual;
            f32 minDepthBounds = 0.0f;
            f32 maxDepthBounds = 1.0f;

            PYRO_NODISCARD  bool operator==(const DepthTestInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const DepthTestInfo&) const = default;
        };

        /**
         * @brief Parameters for configuring stencil testing behavior.
         */
        struct StencilTestInfo {
            /**
             * @brief Operation on the stencil buffer when the stencil test fails.
             */
            StencilOp failOp = StencilOp::Keep;
            /**
             * @brief Operation on the stencil buffer when the stencil test passes.
             */
            StencilOp passOp = StencilOp::Keep;
            /**
             * @brief Operation on the stencil buffer when the depth test fails.
             */
            StencilOp depthFailOp = StencilOp::Keep;
            /**
             * @brief Stencil test compare operation
             */
            CompareOp compareOp = CompareOp::Always;
            /**
             * @brief Stencil test compare mask. This is a bit mask dictating which stencil bits
             * are used for the compare operation.
             */
            u32 compareMask = 0x00000000;
            /**
             * @brief Stencil test write mask. This is a bit mask dictating which stencil bits
             * can be written to.
             */
            u32 writeMask = 0xFFFFFFFF;
            /**
             * @brief Stencil test reference mask. This is a bit mask dictating which stencil bits
             * can be referenced when performing a stencil operation.
             */
            u32 reference = 0xFFFFFFFF;

            PYRO_NODISCARD  bool operator==(const StencilTestInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const StencilTestInfo&) const = default;
        };

        /**
         * @brief Parameters for configuring depth/stencil state in a graphics pipeline.
         */
        struct DepthStencilStateInfo {
            /**
             * @brief Matching render target format for the depth-stencil attachment. *MUST* not be `Undefined`
             */
            Format depthStencilFormat = Format::Undefined;
            /**
             * @brief What the depth test is allowed to do
             */
            DepthStencilTestState depthTestState = DepthStencilTestState::Ignore;
            /**
             * @brief What the stencil test is allowed to do
             */
            DepthStencilTestState stencilTestState = DepthStencilTestState::Ignore;
            /**
             * @brief Depth test information
             */
            DepthTestInfo depthTest = {};
            /**
             * @brief Stencil test information for front facing fragments
             * @related Winding order dicatates what a front face is.
             */
            StencilTestInfo frontStencilTest = {};
            /**
             * @brief Stencil test information for back facing fragments
             * @related Winding order dicatates what a back face is.
             */
            StencilTestInfo backStencilTest = {};

            PYRO_NODISCARD  bool operator==(const DepthStencilStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const DepthStencilStateInfo&) const = default;
        };

        /**
         * @brief Parameters for input assembly configuration in a graphics pipeline.
         */
        struct InputAssemblyStateInfo {
            /**
             * @brief Primitive topology describing how vertices are assembled
             *        (e.g., triangle list, line strip, point list).
             */
            PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;

            /**
             * @brief Type of indices used when drawing indexed primitives.
             */
            IndexType indexType = IndexType::Uint32;

            /**
             * @brief Enables primitive restart when set to true.
             *        Special index values indicate a new primitive without starting a new draw call.
             */
            bool bPrimitiveRestart = false;

            /**
             * @brief Vertex attribute layout descriptions, specifying how vertex
             *        input attributes are interpreted (e.g., position, normals, UVs).
             */
            eastl::span<const VertexAttributeInfo> vertexAttributes = {};

            /**
             * @brief Vertex binding descriptions, specifying the stride and
             *        binding index for each vertex buffer used in the pipeline.
             */
            eastl::span<const VertexBindingInfo> vertexBindings = {};

            PYRO_NODISCARD  bool operator==(const InputAssemblyStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const InputAssemblyStateInfo&) const = default;
        };

        /**
         * @brief Parameters for configuring rasterization behavior in a graphics pipeline.
         */
        struct RasterizerStateInfo {
            /**
             * @brief Determines how primitives are rasterized (e.g., solid triangles, wireframe, points).
             */
            PolygonMode polygonMode = PolygonMode::Triangle;

            /**
             * @brief Line width used when rasterizing lines (only applies if drawing lines).
             * @note Certain APIs outright do not support line widths other than 1.0. Query device limits for min and max widths.
             */
            f32 lineWidth = 1.0f;

            /**
             * @brief Rendering mode used when nrasterizing lines (only applies if drawing lines).
             */
            LineMode lineMode = LineMode::Normal;

            /**
             * @brief Orientation of front-facing polygons (clockwise or counter-clockwise).
             */
            WindingOrder frontFaceWinding = WindingOrder::CounterClockwise;

            /**
             * @brief Which polygon faces to cull, if any (e.g., none, front, back).
             */
            FaceCull faceCulling = FaceCull::None;

            /**
             * @brief If true, clamps depth values to the valid range instead of discarding.
             */
            bool bDepthClamp = false;

            /**
             * @brief Enables depth bias for adjusting fragment depth values.
             */
            bool bDepthBias = false;

            /**
             * @brief Constant factor applied to the depth bias if enabled.
             */
            f32 depthBiasConstantFactor = 0.0f;

            /**
             * @brief Maximum depth bias applied when enabled.
             */
            f32 depthBiasClamp = 0.0f;

            /**
             * @brief Slope factor applied to depth bias for slope-scaled biasing.
             */
            f32 depthBiasSlopeFactor = 0.0f;

            /**
             * @brief If true, disables rasterization entirely (no pixels are generated).
             */
            bool bRasterizerDiscard = false;

            PYRO_NODISCARD  bool operator==(const RasterizerStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const RasterizerStateInfo&) const = default;
        };

        /**
         * @brief Parameters for configuring multisampling in a graphics pipeline.
         */
        struct MultiSampleStateInfo {
            /**
             * @brief Number of samples per pixel for multisample anti-aliasing (MSAA).
             */
            RasterizationSamples sampleCount = RasterizationSamples::e1;

            /**
             * @brief Enables alpha-to-coverage, converting alpha values into
             *        per-sample coverage masks for transparency effects.
             * @note Coverage pattern is implementation defined, certain devices may use
             * dither patterns for coverage, some may truncate sample masks.
             */
            bool bAlphaToCoverage = false;

            /**
             * @brief Enables per-sample shading for improved quality in MSAA.
             */
            bool bSampleShading = false;

            /**
             * @brief Minimum fraction of samples that must be shaded when sample shading is enabled.
             * If bSampleShading is set to `false` and `sampleCount` is greater than 1, minSampleShading must be set to 0.0
             */
            float minSampleShading = 1.0f;

            PYRO_NODISCARD  bool operator==(const MultiSampleStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const MultiSampleStateInfo&) const = default;
        };

        /**
         * @brief Blend state for a single render target.
         */
        struct BlendInfo {
            /**
             * @brief Color blend operation used to combine source and destination colors.
             */
            BlendOp colorBlendOp = BlendOp::Add;

            /**
             * @brief Blend factor applied to source colors before blending.
             */
            BlendFactor srcColorBlendFactor = BlendFactor::One;

            /**
             * @brief Blend factor applied to destination colors before blending.
             */
            BlendFactor dstColorBlendFactor = BlendFactor::Zero;

            /**
             * @brief Blend operation for combining source and destination alpha values.
             */
            BlendOp alphaBlendOp = BlendOp::Add;

            /**
             * @brief Blend factor applied to source alpha values before blending.
             */
            BlendFactor srcAlphaBlendFactor = BlendFactor::One;

            /**
             * @brief Blend factor applied to destination alpha values before blending.
             */
            BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;

            PYRO_NODISCARD  friend auto operator<=>(BlendInfo const&, BlendInfo const&) = default;
        };

        /**
         * @brief Parameters for configuring a single color render target in a graphics pipeline.
         */
        struct ColorTargetStateInfo {
            /**
             * @brief Pixel format of the render target. *MUST* not be `Undefined`
             */
            Format format = Format::Undefined;

            /**
             * @brief Bitmask controlling which color components are written (e.g., RGBA).
             * @note Setting write mask to values different from either the format write mask or 0
             * may be less optimal, as the device may need to perform extra operations to load the previous
             * colour target information, and to store it again.
             */
            ColorComponentFlags writeMask = ColorComponentFlagBits::ALL;

            /**
             * @brief Optional blending parameters for this render target.
             *        If unset, blending is disabled.
             */
            eastl::optional<BlendInfo> blend = eastl::nullopt;

            PYRO_NODISCARD  bool operator==(const ColorTargetStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const ColorTargetStateInfo&) const = default;
        };

        /**
         * @brief Parameters for tessellation state in a graphics pipeline.
         */
        struct TesselationStateInfo {
            /**
             * @brief Number of control points per tessellation patch.
             */
            u32 controlPoints = 3;

            /**
             * @brief Origin for tessellation domain coordinates (e.g., lower-left or upper-left).
             */
            TesselationDomainOrigin origin = TesselationDomainOrigin::LowerLeft;

            PYRO_NODISCARD  bool operator==(const TesselationStateInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const TesselationStateInfo&) const = default;
        };

        /**
         * @brief Parameters for creating a raster (graphics) pipeline.
         */
        struct RasterPipelineInfo {
            /**
             * @brief Render target configurations, one per bound color attachment.
             */
            eastl::fixed_vector<ColorTargetStateInfo, Limits::MAX_BOUND_COLOR_TARGETS> colorTargetStates = {};

            /**
             * @brief Optional depth and stencil test configuration.
             */
            eastl::optional<DepthStencilStateInfo> depthStencilState = {};

            /**
             * @brief Optional tessellation state configuration.
             */
            eastl::optional<TesselationStateInfo> tesselationState = {};

            /**
             * @brief Input assembly state specifying how vertices are interpreted.
             */
            InputAssemblyStateInfo inputAssemblyState = {};

            /**
             * @brief Rasterization state controlling polygon rendering and culling.
             */
            RasterizerStateInfo rasterizerState = {};

            /**
             * @brief Multisampling state controlling anti-aliasing behavior.
             */
            MultiSampleStateInfo multiSampleState = {};

            /**
             * @brief Optional debug name for this pipeline.
             */
            eastl::string name = {};

            PYRO_NODISCARD  bool operator==(const RasterPipelineInfo&) const = default;
            PYRO_NODISCARD  bool operator!=(const RasterPipelineInfo&) const = default;
        };

        struct RasterPipelineShaderStages {
            /**
             * @brief Vertex shader stage configuration.
             */
            eastl::optional<ShaderInfo> vertexShaderInfo = {};

            /**
             * @brief Hull (tessellation control) shader stage configuration.
             */
            eastl::optional<ShaderInfo> hullShaderInfo = {};

            /**
             * @brief Domain (tessellation evaluation) shader stage configuration.
             */
            eastl::optional<ShaderInfo> domainShaderInfo = {};

            /**
             * @brief Geometry shader stage configuration.
             */
            eastl::optional<ShaderInfo> geometryShaderInfo = {};

            /**
             * @brief Fragment (pixel) shader stage configuration.
             */
            eastl::optional<ShaderInfo> fragmentShaderInfo = {};

            PYRO_NODISCARD  bool operator==(const RasterPipelineShaderStages&) const = default;
            PYRO_NODISCARD  bool operator!=(const RasterPipelineShaderStages&) const = default;
        };

        /**
         * @brief Handle representing a raster (graphics) pipeline object.
         */
        RHI_TYPED_PTR_HANDLE(RasterPipeline);
    } // namespace RHI
} // namespace PyroshockStudios