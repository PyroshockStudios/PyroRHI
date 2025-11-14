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
#include <EASTL/span.h>
#include <EASTL/string.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <PyroRHI/Api/Types.hpp>


namespace PyroshockStudios {
    inline namespace RHI {

        struct AccelerationStructureCreateFlagsProperties {
            using Data = u32;
        };
        using AccelerationStructureCreateFlags = Flags<AccelerationStructureCreateFlagsProperties>;
        struct AccelerationStructureCreateFlagBits {
            static constexpr inline AccelerationStructureCreateFlags NONE = { 0x00000000 };
            static constexpr inline AccelerationStructureCreateFlags ALLOW_UPDATE = { 0x00000001 };
            static constexpr inline AccelerationStructureCreateFlags ALLOW_COMPACTION = { 0x00000002 };
            static constexpr inline AccelerationStructureCreateFlags PREFER_FAST_TRACE = { 0x00000004 };
            static constexpr inline AccelerationStructureCreateFlags PREFER_FAST_BUILD = { 0x00000008 };
            static constexpr inline AccelerationStructureCreateFlags LOW_MEMORY = { 0x00000010 };
            static constexpr inline AccelerationStructureCreateFlags ALLOW_DATA_ACCESS = { 0x00000800 };
        };


        struct AccelerationStructureGeometryFlagsProperties {
            using Data = u32;
        };
        using AccelerationStructureGeometryFlags = Flags<AccelerationStructureGeometryFlagsProperties>;
        struct AccelerationStructureGeometryFlagBits {
            static constexpr inline AccelerationStructureGeometryFlags NONE = { 0x00000000 };
            static constexpr inline AccelerationStructureGeometryFlags OPAQUE = { 0x00000001 };
            static constexpr inline AccelerationStructureGeometryFlags NO_DUPLICATE_ANY_HIT_INVOCATION = { 0x00000002 };
        };

        using AccelerationStructureGeometryInstanceFlags = u32;
        struct AccelerationStructureGeometryInstanceFlagBits {
            static constexpr inline AccelerationStructureGeometryInstanceFlags NONE = { 0x00 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags TRIANGLE_FACING_CULL_DISABLE = { 0x01 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags TRIANGLE_FLIP_FACING = { 0x02 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags FORCE_OPAQUE = { 0x04 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags FORCE_NO_OPAQUE = { 0x08 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags FORCE_OPACITY_MICROMAP_STATE = { 0x10 };
            static constexpr inline AccelerationStructureGeometryInstanceFlags DISABLE_OPACITY_MICROMAPS = { 0x20 };
        };

        struct BlasTriangleGeometryInfo {
            Format vertexFormat = Format::Undefined; ///< Required, vertex format. How the BLAS should interpret the vertices.
            IndexType indexType = IndexType::None;   ///< Optional index format, if type is NONE, indexBuffer is expected to be PYRO_NULL_BUFFER
            Buffer vertexBuffer = PYRO_NULL_BUFFER;  ///< Required, vertex data.
            Buffer indexBuffer = PYRO_NULL_BUFFER;   ///< Optional index data
            u32 vertexStride = 0;                    ///< Number of bytes to get to the next vertex
            u32 vertexByteOffset = 0;                ///< Byte offset into the vertiex buffer.
            u32 indexOffset = 0;                     ///< Amount of indices to skip.
            u32 vertexCount = 0;                     ///< Total number of vertices to include.
            u32 indexCount = 0;                      ///< Total number of indices to include.
            AccelerationStructureGeometryFlags flags = AccelerationStructureGeometryFlagBits::OPAQUE;
            Buffer transformData = PYRO_NULL_BUFFER; ///< Optional, transforms all vertices by this matrix.
            DeviceSize transformDataOffset = {};     ///< Byte offset into the transform data buffer.
        };

        struct BlasAabbGeometryInfo {
            Buffer data = PYRO_NULL_BUFFER;
            u32 stride = {};
            u32 count = {};
            AccelerationStructureGeometryFlags flags = AccelerationStructureGeometryFlagBits::OPAQUE; ///< Attribute for opaque/transparent geometry
        };

        using BlasAddress = u64;

        /**
         * @brief Parameters used to create a bottom-level acceleration structure for ray tracing.
         */
        struct BlasInfo {
            /// @brief Memory size in bytes
            DeviceSize size = {};
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};
            PYRO_NODISCARD bool operator==(const BlasInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BlasInfo&) const = default;
        };

        /**
         * @brief GPU Blas handle.
         */
        struct BlasId : public GpuResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const BlasId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const BlasId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Null (invalid) bottom-level acceleration structure handle.
        constexpr BlasId PYRO_NULL_BLAS = BlasId{};

        constexpr BlasAddress PYRO_NULL_BLAS_ADDRESS = 0;

        struct BlasBuildInfo {
            AccelerationStructureCreateFlags flags = AccelerationStructureCreateFlagBits::NONE;
            bool bUpdate = false;
            BlasId srcBlas = PYRO_NULL_BLAS;
            BlasId dstBlas = PYRO_NULL_BLAS;
            eastl::variant<eastl::span<const BlasTriangleGeometryInfo>, eastl::span<const BlasAabbGeometryInfo>> geometries = {};
            Buffer scratchBuffer = PYRO_NULL_BUFFER;
        };

        struct BlasInstanceData {
            Transform transform = Transform::IDENTITY;
            u32 instanceCustomIndex : 24;
            u32 mask : 8;
            u32 instanceShaderBindingTableRecordOffset : 24;
            AccelerationStructureGeometryInstanceFlags flags : 8;
            BlasAddress blasAddress = PYRO_NULL_BLAS_ADDRESS;
        };

        static_assert(sizeof(BlasInstanceData) == 64, "BlasInstanceData is incorrectly layed out");

        struct TlasInstanceInfo {
            Buffer data = {};
            u32 count = {};
            // bool bDataArrayOfPointers = {};
            AccelerationStructureGeometryFlags flags = AccelerationStructureGeometryFlagBits::OPAQUE; ///< Attribute for opaque/transparent instance
        };

        /**
         * @brief Parameters used to create a top-level acceleration structure for ray tracing.
         */
        struct TlasInfo {
            /// @brief Memory size in bytes
            DeviceSize size = {};
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const TlasInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const TlasInfo&) const = default;
        };

        /**
         * @brief Identifier for a top-level acceleration structure (Tlas).
         */
        struct TlasId : public GpuResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const TlasId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const TlasId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Null (invalid) top-level acceleration structure handle.
        constexpr TlasId PYRO_NULL_TLAS = TlasId{};

        struct TlasBuildInfo {
            AccelerationStructureCreateFlags flags = AccelerationStructureCreateFlagBits::NONE;
            bool update = false;
            TlasId srcTlas = PYRO_NULL_TLAS;
            TlasId dstTlas = PYRO_NULL_TLAS;
            TlasInstanceInfo instances = {};
            Buffer scratchBuffer = PYRO_NULL_BUFFER;
        };

        struct AccelerationStructureBuildSizesInfo {
            DeviceSize accelerationStructureSize;
            DeviceSize updateScratchSize;
            DeviceSize buildScratchSize;
        };

    } // namespace RHI
} // namespace PyroshockStudios

namespace eastl {
    using namespace PyroshockStudios;
    using namespace PyroshockStudios::RHI;

    template <>
    struct hash<BlasId> {
        usize operator()(BlasId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
    template <>
    struct hash<TlasId> {
        usize operator()(TlasId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
} // namespace eastl