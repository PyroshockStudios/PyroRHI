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
#include <EASTL/bit.h>
#include <EASTL/compare.h>
#include <EASTL/functional.h>
#include <EASTL/optional.h>
#include <EASTL/string.h>

#include "Types.hpp"
#include <PyroRHI/Core.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        struct IDevice;

        constexpr u32 PYRO_NULL_DESCRIPTOR = 0U;

        /// @brief Handle to GPU memory.
        RHI_TYPED_HANDLE64(MemoryBlock);

        /// @brief Handle to a GPU buffer resource.
        RHI_TYPED_HANDLE64(Buffer);

        /// @brief Handle to a GPU image resource.
        RHI_TYPED_HANDLE64(Image);

        /// @brief Null (invalid) memory block handle.
        constexpr MemoryBlock PYRO_NULL_MEMORY_BLOCK = MemoryBlock{};

        /// @brief Null (invalid) buffer handle.
        constexpr Buffer PYRO_NULL_BUFFER = Buffer{};

        /// @brief Null (invalid) image handle.
        constexpr Image PYRO_NULL_IMAGE = Image{};


        //-------------------------------------------------------------------------------------------------
        // Virtual Memory Block Creation Structure
        //-------------------------------------------------------------------------------------------------

        /**
         * @brief Suballocation strategy for virtual allocations within a MemoryBlock
         */
        enum struct VirtualSuballocationStrategy : i32 {
            Default = 0,        /**< Default implementation */
            SpaceEfficient = 1, /**< Space efficient suballocation mode. Might sacrifice suballocation performance */
            TimeEfficient = 2,  /**< Fast suballocation mode. Might sacrifice space usage */
            AggressiveRing = 3, /**< Fastest suballocation mode. This will severely sacrifice space usage. Not recommended for anything except for ring buffers, where spaces are FiFo recycled*/
        };

        /**
         * @brief Parameters for creating a GPU memory block for virtual allocations.
         */
        struct MemoryBlockInfo {
            BufferUsageFlags bufferUsage = {};                                             /**< Intended possible usages of subsequent buffers (e.g., vertex, uniform). */
            ImageUsageFlags imageUsage = {};                                               /**< Intended possible usages of subsequent images (e.g., sampled, unordered access). */
            DeviceSize size = {};                                                          /**< Memory size in bytes. */
            VirtualSuballocationStrategy strategy = VirtualSuballocationStrategy::Default; /**< Strategy used by resources creating allocations. */
            MemoryAllocationDomain domain = MemoryAllocationDomain::DeviceLocal;           /**< Memory Block Domain. */
            u32 minAlignment = 1;                                                          /**< Minimum suballocation alignment. *MUST* be a multiple of 2*/
            eastl::string name = {};                                                       /**< Optional debug name for the memory handle. */

            PYRO_NODISCARD bool operator==(const MemoryBlockInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const MemoryBlockInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        //-------------------------------------------------------------------------------------------------
        // Buffer Creation Structures
        //-------------------------------------------------------------------------------------------------

        struct BufferCreateFlagsProperties {
            using Data = u32;
        };

        using BufferCreateFlags = Flags<BufferCreateFlagsProperties>;

        /**
         * @brief Buffer creation flag bits.
         */
        struct BufferCreateFlagsBits {
            static inline constexpr BufferCreateFlags NONE = { 0x00000000 };        /**< No flags. */
            static inline constexpr BufferCreateFlags ALLOW_ALIAS = { 0x00000001 }; /**< Allows aliasing buffer memory. */
        };

        /**
         * @brief Parameters for creating a GPU buffer.
         */
        struct BufferInfo {
            /**
             * @brief Optional memory block handle for virtual allocations.
             * If `memoryBlock` is `PYRO_NULL_MEMORY_BLOCK`, then the buffer will create its own allocation handle.
             */
            MemoryBlock memoryBlock = PYRO_NULL_MEMORY_BLOCK;
            BufferCreateFlags flags = BufferCreateFlagsBits::NONE; /**< Buffer creation flags. */
            DeviceSize size = {};                                  /**< Buffer size in bytes. */
            BufferUsageFlags usage = {};                           /**< Intended usage of the buffer (e.g., vertex, uniform). */
            BufferLayout initialLayout = BufferLayout::Undefined;  /**< Initial state of the buffer. */
            /**
             * @brief Memory allocation domain.
             * If `memoryPool` is not `PYRO_NULL_MEMORY_BLOCK`, then `allocationDomain` will be ignored.
             */
            MemoryAllocationDomain allocationDomain = MemoryAllocationDomain::DeviceLocal;
            eastl::string name = {}; /**< Optional debug name for the buffer. */

            PYRO_NODISCARD bool operator==(const BufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        //-------------------------------------------------------------------------------------------------
        // Image Creation Structures
        //-------------------------------------------------------------------------------------------------

        struct ImageCreateFlagsProperties {
            using Data = u32;
        };

        using ImageCreateFlags = Flags<ImageCreateFlagsProperties>;

        /**
         * @brief Image creation flag bits.
         */
        struct ImageCreateFlagBits {
            static inline constexpr ImageCreateFlags NONE = { 0x00000000 };           /**< No flags. */
            static inline constexpr ImageCreateFlags MUTABLE_FORMAT = { 0x00000001 }; /**< Allows different formats when creating UAVs or SRVs. */
            static inline constexpr ImageCreateFlags CUBE = { 0x00000002 };           /**< Create a cube-compatible image. */
            static inline constexpr ImageCreateFlags ALLOW_ALIAS = { 0x00000004 };    /**< Allows aliasing image memory. */
        };

        enum struct ImageDimensions : u32 {
            e1D = 1,
            e2D = 2,
            e3D = 3
        };

        /**
         * @brief Parameters for creating a GPU image.
         */
        struct ImageInfo {
            /**
             * @brief Optional memory block handle for virtual allocations.
             * If `memoryBlock` is `PYRO_NULL_MEMORY_BLOCK`, then the image will create its own allocation handle.
             */
            MemoryBlock memoryBlock = PYRO_NULL_MEMORY_BLOCK;
            ImageCreateFlags flags = ImageCreateFlagBits::NONE;          /**< Image creation flags. */
            ImageDimensions dimensions = ImageDimensions::e2D;           /**< Number of dimensions (1D, 2D, 3D). */
            Format format = Format::RGBA8Unorm;                          /**< Pixel format of the image. */
            Extent3D size = {};                                          /**< Dimensions of the image (width, height, depth). */
            u32 mipLevelCount = 1;                                       /**< Number of mipmap levels. */
            u32 arrayLayerCount = 1;                                     /**< Number of array layers. */
            RasterizationSamples sampleCount = RasterizationSamples::e1; /**< Number of samples per pixel (for MSAA). */
            ImageUsageFlags usage = {};                                  /**< Intended usage of the image (e.g., render target, sampling). */
            eastl::string name = {};                                     /**< Optional debug name for the image. */

            PYRO_NODISCARD bool operator==(const ImageInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ImageInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        //-------------------------------------------------------------------------------------------------
        // Sampler Configuration
        //-------------------------------------------------------------------------------------------------

        /**
         * @brief Parameters for configuring a texture sampler.
         */
        struct SamplerInfo {
            Filter magnificationFilter = Filter::Linear;                       /**< Filter for magnifying textures. */
            Filter minificationFilter = Filter::Linear;                        /**< Filter for minifying textures. */
            Filter mipmapFilter = Filter::Linear;                              /**< Filter for sampling between mipmap levels. */
            ReductionMode reductionMode = ReductionMode::WeightedAverage;      /**< How multiple samples are reduced. */
            SamplerAddressMode addressModeU = SamplerAddressMode::ClampToEdge; /**< Addressing mode for U coordinate. */
            SamplerAddressMode addressModeV = SamplerAddressMode::ClampToEdge; /**< Addressing mode for V coordinate. */
            SamplerAddressMode addressModeW = SamplerAddressMode::ClampToEdge; /**< Addressing mode for W coordinate. */
            f32 mipLodBias = 0.0f;                                             /**< Bias applied to the mipmap LOD level. */
            bool enableAnisotropy = false;                                     /**< Enables anisotropic filtering. */
            u32 maxAnisotropy = 1;                                             /**< Maximum anisotropy if enabled. */
            bool enableCompare = false;                                        /**< Enables depth comparison for shadow mapping. */
            CompareOp compareOp = CompareOp::Never;                            /**< Comparison operation if enabled. */
            f32 minLod = 0.0f;                                                 /**< Minimum mipmap LOD. */
            f32 maxLod = FLT_MAX;                                              /**< Maximum mipmap LOD. */
            BorderColor borderColor = BorderColor::TransparentBlackFloat;      /**< Border color for texture sampling. */
            eastl::string name = {};                                           /**< Optional debug name for the sampler. */

            PYRO_NODISCARD bool operator==(const SamplerInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SamplerInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        //-------------------------------------------------------------------------------------------------
        // GPU Resource Identifiers
        //-------------------------------------------------------------------------------------------------

        /**
         * @brief Generic GPU resource identifier with index and version.
         */
        struct GpuResourceId {
            u32 index = PYRO_NULL_DESCRIPTOR; /**< Resource index in a table. */
            u32 version = 0x00000000;         /**< Version number to avoid stale references. */

            /**
             * @brief Three-way comparison operator.
             */
            PYRO_FORCEINLINE auto operator<=>(GpuResourceId const& other) const {
                return eastl::bit_cast<u64>(*this) <=> eastl::bit_cast<u64>(other);
            }
        };

        /// @brief Identifier for a shader resource view (SRV).
        struct ShaderResourceId : public GpuResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const ShaderResourceId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const ShaderResourceId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Identifier for an unordered access view (UAV).
        /// @note The `index` variable is not to be used as a bindless index.
        struct UnorderedAccessId : public GpuResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const UnorderedAccessId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const UnorderedAccessId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Identifier for a sampler resource.
        struct SamplerId : public GpuResourceId {
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const SamplerId& other) const {
                return eastl::bit_cast<u64>(*this) == eastl::bit_cast<u64>(other);
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const SamplerId& other) const {
                return !(*this == other);
            }
        };

        /// @brief Null SRV identifier.
        constexpr ShaderResourceId PYRO_NULL_SRV = ShaderResourceId{};

        /// @brief Null UAV identifier.
        constexpr UnorderedAccessId PYRO_NULL_UAV = UnorderedAccessId{};

        /// @brief Null sampler identifier.
        constexpr SamplerId PYRO_NULL_SAMPLER = SamplerId{};

        //-------------------------------------------------------------------------------------------------
        // Resource View Structures
        //-------------------------------------------------------------------------------------------------

        /**
         * @brief Buffer resource view information.
         */
        struct BufferResourceInfo {
            Buffer buffer = PYRO_NULL_BUFFER; /**< Buffer handle. */
            BufferRegion region = {};         /**< Region of the buffer to be used. */
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Types of image views that can be created.
         */
        enum struct ImageViewType : i32 {
            e1D = 0,       /**< 1D texture view. */
            e2D = 1,       /**< 2D texture view. */
            e3D = 2,       /**< 3D texture view. */
            eCube = 3,     /**< Cube texture view. */
            e1DArray = 4,  /**< 1D array texture view. */
            e2DArray = 5,  /**< 2D array texture view. */
            eCubeArray = 6 /**< Cube array texture view. */
        };

        /**
         * @brief Image resource view information.
         */
        struct ImageResourceInfo {
            Image image = PYRO_NULL_IMAGE; /**< Image handle. */
            /**
             * @brief  Subresource slice (mips + array levels).
             * @note If creating an Unordered Access View, only *ONE* mip level is allowed to be selected.
             */
            ImageMipArraySlice slice = {};
            ImageViewType viewType = ImageViewType::e2D; /**< Type of image view. */
            Format format = Format::Inherit;             /**< Format override, if any. */
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /// @brief Variant type holding either a buffer or image resource view.
        /// Used for SRV/UAV creation
        using GpuResourceInfo = eastl::variant<BufferResourceInfo, ImageResourceInfo>;

    } // namespace RHI
} // namespace PyroshockStudios

namespace eastl {
    using namespace PyroshockStudios;
    using namespace PyroshockStudios::RHI;

    template <>
    struct hash<GpuResourceId> {
        usize operator()(GpuResourceId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
    template <>
    struct hash<ShaderResourceId> {
        usize operator()(ShaderResourceId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
    template <>
    struct hash<UnorderedAccessId> {
        usize operator()(UnorderedAccessId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
    template <>
    struct hash<SamplerId> {
        usize operator()(SamplerId k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };

    template <>
    struct hash<MemoryBlock> {
        usize operator()(MemoryBlock k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };

    template <>
    struct hash<Buffer> {
        usize operator()(Buffer k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
    template <>
    struct hash<Image> {
        usize operator()(Image k) const {
            return eastl::hash<usize>{}(eastl::bit_cast<u64>(k));
        }
    };
} // namespace eastl
