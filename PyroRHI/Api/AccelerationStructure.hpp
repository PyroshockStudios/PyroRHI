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
#include <EASTL/string.h>
#include <EASTL/span.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/Types.hpp>
#include <PyroRHI/Api/GPUResource.hpp>

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

        enum struct GeometryFlags {
            Opaque = 0x00000001,
            NO_DUPLICATE_ANY_HIT_INVOCATION = 0x00000002,
        };

        struct BLASTriangleGeometryInfo {
            Format vertexFormat = Format::Undefined;
            IndexType indexType = IndexType::Uint32;
            Buffer vertexBuffer = PYRO_NULL_BUFFER; // vertex data
            Buffer indexBuffer = PYRO_NULL_BUFFER;  // optional, only for triangles
            u32 vertexStride = 0;
            u32 vertexOffset = 0;
            u32 indexOffset = 0;
            u32 vertexCount = 0;
            u32 indexCount = 0;
            GeometryFlags flags = GeometryFlags::Opaque;  ///< Attribute for opaque/transparent geometry
            Buffer transformData = PYRO_NULL_BUFFER;
            usize transformDataOffset = {};
        };

        struct BLASAABBGeometryInfo {
            Buffer data = PYRO_NULL_BUFFER;
            u32 stride = {};
            u32 count = {};
            GeometryFlags flags = GeometryFlags::Opaque;  ///< Attribute for opaque/transparent geometry
        };

        /**
         * @brief Parameters used to create a bottom-level acceleration structure for ray tracing.
         */
        struct BLASInfo {
            /// @brief Memory size in bytes
            DeviceSize size = {};  
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};
            PYRO_NODISCARD bool operator==(const BLASInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BLASInfo&) const = default;
        };

        /**
         * @brief GPU BLAS handle.
         */
        struct BLASId : public GPUResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const BLASId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const BLASId& other) const {
                return !(*this == other);
            }
        };
        
        /// @brief Null (invalid) bottom-level acceleration structure handle.
        constexpr BLASId PYRO_NULL_BLAS = BLASId{};

        struct BLASBuildInfo {
            AccelerationStructureCreateFlags flags = AccelerationStructureCreateFlagBits::NONE;
            bool bUpdate = false;
            BLASId srcBLAS = PYRO_NULL_BLAS;
            BLASId dstBLAS = PYRO_NULL_BLAS;
            eastl::variant<eastl::span<const BLASTriangleGeometryInfo>, eastl::span<const BLASAABBGeometryInfo>> geometries = {};
            Buffer scratchBuffer = PYRO_NULL_BUFFER;
        };

        struct BlasInstanceData {
            BLASId blas = PYRO_NULL_BLAS; // TODO: You need a adress and not blasId
            Transform transform = Transform::IDENTITY;  
            u32 instanceCustomIndex : 24;
            u32 mask : 8;
            u32 instanceShaderBindingTableRecordOffset : 24;
            u32 flags : 8;
        };

        struct TLASInstanceInfo {
            Buffer data = {};
            u32 count = {};
            bool isDataArrayOfPointers = {};
            GeometryFlags flags = GeometryFlags::Opaque;  ///< Attribute for opaque/transparent instance
        };

        /**
         * @brief Parameters used to create a top-level acceleration structure for ray tracing.
         */
        struct TLASInfo {
            /// @brief Memory size in bytes
            DeviceSize size = {}; 
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const TLASInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const TLASInfo&) const = default;
        };
        
        /**
         * @brief Identifier for a top-level acceleration structure (TLAS).
         */
        struct TLASId : public GPUResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const TLASId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const TLASId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Null (invalid) top-level acceleration structure handle.
        constexpr TLASId PYRO_NULL_TLAS = TLASId{};

        struct TLASBuildInfo {
            AccelerationStructureCreateFlags flags = AccelerationStructureCreateFlagBits::NONE;
            bool update = false;
            TLASId srcTlas = PYRO_NULL_TLAS;
            TLASId dstTlas = PYRO_NULL_TLAS;
            eastl::span<const TLASInstanceInfo> instances = {};
            Buffer scratchBuffer = PYRO_NULL_BUFFER;
        };

        struct AccelerationStructureBuildSizesInfo {
            DeviceSize accelerationStructureSize;
            DeviceSize updateScratchSize;
            DeviceSize buildScratchSize;
        };

    } // namespace RHI
} // namespace PyroshockStudios
