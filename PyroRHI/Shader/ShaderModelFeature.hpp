#pragma once
#include <PyroCommon/Types.hpp>
namespace PyroshockStudios {
    inline namespace RHI {
        struct ShaderModelFeatureFlagProperties {
            using Data = u64;
        };
        using ShaderModelFeatureFlags = Flags<ShaderModelFeatureFlagProperties>;
        struct ShaderModelFeatureBits {
            // --- Baseline Features (SM 5.1 / SPIR-V 1.3+) ---

            /** * @brief Wave Intrinsics (e.g., WaveGetLaneIndex, WaveActiveSum).
             * - HLSL: Shader Model 6.0+
             * - SPIR-V: 1.3+ (Core in Vulkan 1.1) with GroupNonUniform capabilities.
             */
            static constexpr inline ShaderModelFeatureFlags SUBGROUP_OPERATIONS = { 0x0000000001 };

            /**
             * @brief 64-bit integer types (uint64_t, int64_t).
             * - HLSL: Shader Model 6.0+
             * - SPIR-V: Requires 'Int64' capability.
             */
            static constexpr inline ShaderModelFeatureFlags SHADER_INT64 = { 0x0000000002 };

            /**
             * @brief 16-bit float types (half, float16_t).
             * - HLSL: Shader Model 6.0+ (and 6.2+ for more ops)
             * - SPIR-V: Requires 'Float16' capability.
             */
            static constexpr inline ShaderModelFeatureFlags SHADER_FLOAT16 = { 0x0000000004 };

            /**
             * @brief Conservative Rasterization.
             * - HLSL: Shader Model 6.0+ (Tied to API capability)
             * - SPIR-V: Requires 'SPV_EXT_conservative_rasterization' extension.
             */
            static constexpr inline ShaderModelFeatureFlags CONSERVATIVE_RASTERIZATION = { 0x0000000008 };

            /**
             * @brief Ray Tracing (Gen Shaders, Hit Shaders, etc.).
             * - HLSL: Shader Model 6.3+ (DXR)
             * - SPIR-V: 1.4+ (Requires 'SPV_KHR_ray_tracing' or similar extension)
             */
            static constexpr inline ShaderModelFeatureFlags RAY_TRACING = { 0x0000000010 };

            /**
             * @brief Inline Ray Tracing (TraceRaysInline).
             * - HLSL: Shader Model 6.5+
             * - SPIR-V: 1.4+ (Requires 'SPV_KHR_ray_query' or similar extension)
             */
            static constexpr inline ShaderModelFeatureFlags RAY_QUERY = { 0x0000000020 };

            /**
             * @brief Variable Rate Shading (VRS).
             * - HLSL: Shader Model 6.4+
             * - SPIR-V: 1.5+ (Requires 'SPV_KHR_fragment_shading_rate' extension)
             */
            static constexpr inline ShaderModelFeatureFlags VARIABLE_RATE_SHADING = { 0x0000000040 };

            /**
             * @brief Mesh and Task Shaders.
             * - HLSL: Shader Model 6.5+
             * - SPIR-V: 1.5+ (Requires 'SPV_EXT_mesh_shader' extension)
             */
            static constexpr inline ShaderModelFeatureFlags MESH_SHADER = { 0x0000000080 };

            /**
             * @brief Atomic operations on 64-bit integers.
             * - HLSL: Shader Model 6.6+
             * - SPIR-V: Requires 'AtomicInt64' capability.
             */
            static constexpr inline ShaderModelFeatureFlags SHADER_INT64_ATOMICS = { 0x0000000100 };
        };
    } // namespace RHI
} // namespace PyroshockStudios