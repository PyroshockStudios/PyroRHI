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
        
        struct BLASCreateFlagsProperties {
            using Data = u32;
        };
        using BLASCreateFlags = Flags<BLASCreateFlagsProperties>;
        struct BLASCreateFlagBits {
            static constexpr inline BLASCreateFlags NONE = { 0x00000000 };
            static constexpr inline BLASCreateFlags ALLOW_REBUILD = { 0x00000001 };
            static constexpr inline BLASCreateFlags PREFER_FAST_TRACE = { 0x00000002 };
            static constexpr inline BLASCreateFlags PREFER_FAST_BUILD = { 0x00000004 };
        };

        enum struct BLASGeometryType : u32 {
            Triangles = 0,
            AABB = 1
        };
        enum struct BLASGeometryOpacity {
            Opaque = 0,
            Transparent = 1,
        };

        struct BLASGeometryInfo {
            BLASGeometryType type = BLASGeometryType::Triangles;
            Format vertexFormat = Format::Undefined;
            IndexType indexType = IndexType::Uint32;
            Buffer vertexBuffer = PYRO_NULL_BUFFER; // vertex data
            Buffer indexBuffer = PYRO_NULL_BUFFER;  // optional, only for triangles
            u32 vertexStride = 0;
            u32 vertexOffset = 0;
            u32 indexOffset = 0;
            u32 vertexCount = 0;
            u32 indexCount = 0;
            BLASGeometryOpacity opacity = BLASGeometryOpacity::Opaque;  ///< Attribute for opaque/transparent geometry
        };

        /**
         * @brief Parameters used to create a bottom-level acceleration structure for ray tracing.
         */
        struct BLASInfo {
            /// @brief Create flags
            BLASCreateFlags flags = BLASCreateFlagBits::NONE;
            /// @brief Optional human-readable name for debugging/profiling.
            eastl::string name = {};
            PYRO_NODISCARD bool operator==(const BLASInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BLASInfo&) const = default;
        };

        /**
         * @brief GPU BLAS handle.
         */
        RHI_TYPED_PTR_HANDLE(BLAS);
        
        /// @brief Null (invalid) bottom-level acceleration structure handle.
        constexpr BLAS PYRO_NULL_BLAS = BLAS{};
        
        struct TLASCreateFlagsProperties {
            using Data = u32;
        };
        using TLASCreateFlags = Flags<TLASCreateFlagsProperties>;
        struct TLASCreateFlagBits {
            static constexpr inline TLASCreateFlags NONE = { 0x00000000 };
            static constexpr inline TLASCreateFlags ALLOW_UPDATE = { 0x00000001 };
            static constexpr inline TLASCreateFlags PREFER_FAST_TRACE = { 0x00000002 };
            static constexpr inline TLASCreateFlags PREFER_FAST_BUILD = { 0x00000004 };
        };

        enum struct TLASInstanceOpacity {
            Opaque = 0,
            Transparent = 1,
        };

        struct TLASInstanceInfo {
            BLAS blas = PYRO_NULL_BLAS;                                 ///< reference to bottom-level AS. *MUST* be NON null
            Transform transform = Transform::IDENTITY;                  ///< world transform
            u32 instanceID = 0;                                         ///< arbitrary identifier
            u32 hitGroupMask = 0xFFFFFFFF;                              ///< which hit groups to consider
            TLASInstanceOpacity opacity = TLASInstanceOpacity::Opaque;  ///< Attribute for opaque/transparent instance
        };

        /**
         * @brief Parameters used to create a top-level acceleration structure for ray tracing.
         */
        struct TLASInfo {
            /// @brief Create flags
            TLASCreateFlags flags = TLASCreateFlagBits::NONE;
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

    } // namespace RHI
} // namespace PyroshockStudios
