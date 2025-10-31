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
#include <PyroCommon/Core.hpp>

#include "AccelerationStructure.hpp"
#include "GPUResource.hpp"
#include "ICommandBuffer.hpp"
#include "ICommandQueue.hpp"
#include "IFence.hpp"
#include "IQueryPool.hpp"
#include "ISwapChain.hpp"
#include "Pipeline.hpp"
#include "Semaphore.hpp"

#include "RenderTarget.hpp"

#include <EASTL/span.h>

namespace PyroshockStudios {
    inline namespace RHI {
        enum struct DeviceType : u32 {
            Unknown = 0,
            Discrete = 1,
            Integrated = 2,
            Virtual = 3,
            CPU = 4
        };
        /**
         * @brief Contains descriptive, vendor, and classification information about a GPU device.
         * Filled once at device initialization.
         */
        struct DeviceInfo {
            // --- Generic identifiers ---
            eastl::string name;   ///< User-friendly GPU name (e.g., "NVIDIA GeForce RTX 4090").
            eastl::string vendor; ///< Vendor name ("NVIDIA", "AMD", "Intel", etc.).

            // --- Vendor / device identifiers ---
            u32 vendorID = 0;    ///< PCI vendor ID (e.g., 0x10DE for NVIDIA).
            u32 deviceID = 0;    ///< PCI device ID (model specific).
            u32 subsystemID = 0; ///< Optional subsystem ID if available.
            u32 revisionID = 0;  ///< Hardware revision or stepping.

            // --- Device class flags ---
            DeviceType deviceType = DeviceType::Unknown; ///< Device type
            bool bUnifiedMemory = false;                 ///< True on UMA architectures (APUs / integrated GPUs).
            bool bRemovable = false;                     ///< True if the device can be hot-removed (eGPU).
            bool bPrimaryAdapter = false;                ///< True if this is the system's primary adapter.
            bool bHeadless = false;                      ///< True if the device supports headless rendering only.

            // --- Driver and API details ---
            eastl::string driverVersion;     ///< Driver version string (parsed from DXGI or Vulkan driver info).
            eastl::string apiVersion;        ///< Graphics API version (e.g., "D3D12.3" or "Vulkan 1.3.290").
            eastl::string driverDescription; ///< Optional additional text from the driver or runtime.
            eastl::string architecture;      ///< e.g. "Ada Lovelace", "RDNA3", "Xe-LPG", etc., if identifiable.

            // --- Hardware limits (optional basic summary) ---
            DeviceSize dedicatedVideoMemory = 0; ///< From DXGI_ADAPTER_DESC or VkPhysicalDeviceMemoryProperties.
            DeviceSize sharedSystemMemory = 0;
            u32 adapterLUIDLow = 0;  ///< Lower 32 bits of the adapter LUID (DX12 only).
            u32 adapterLUIDHigh = 0; ///< Upper 32 bits of the adapter LUID. (DX12 only).

            PYRO_NODISCARD bool operator==(const DeviceInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DeviceInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };


        /**
         * @brief Describes feature support for a given GPU device.
         * This structure is designed to abstract both D3D12 feature queries and Vulkan feature sets.
         */
        struct DeviceFeaturesInfo {
            // --- Shader Stages ---
            bool bGeometryShaders = false;
            bool bTesselationShaders = false;
            bool bMeshShaders = false;
            bool bTaskShaders = false;

            // --- Ray Tracing ---
            bool bRayQueries = false;
            bool bRayTracingPipelines = false;
            bool bAccelerationStructureBuild = false;

            // --- Resource / Memory ---
            bool bBufferDeviceAddress = false; ///< VK_KHR_buffer_device_address / D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT

            // --- Texture Compression ---
            bool bBCnTextureCompression = false;

            // --- Index Buffer Formats ---
            bool bUint8IndexBuffer = false;

            // --- Shader Model / SPIR-V Level ---
            u32 supportedShaderModel = 0; ///< e.g., HLSL Shader Model or Vulkan's SPIR-V version, format of 0xMAJOR_MINOR

            // --- Compute and Atomics ---
            bool bInt64ShaderOps = false;
            bool bAtomicFloatOps = false;  ///< VK_EXT_shader_atomic_float or DX12 SM6.6+
            bool bWaveOps = false;         ///< DX12 Wave Intrinsics or Vulkan subgroup ops
            bool bSubgroupQuadOps = false; ///< Subgroup operations for advanced wave programming

            // --- Presentation / Display ---
            bool bHeadlessSwapChainWindow = false;
            bool bVariableRateShading = false;

            // --- Conservative Rasterization ---
            bool bConservativeRasterization = false; // D3D12 conservative raster / VK_EXT_conservative_rasterization

            PYRO_NODISCARD bool operator==(const DeviceFeaturesInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DeviceFeaturesInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Describes the static hardware properties and alignment constraints of a GPU device.
         * These are fixed per physical device and do not change at runtime.
         */
        struct DevicePropertiesInfo {
            // --- MSAA and sample counts ---
            RasterizationSamples msaaSupportColorTarget = RasterizationSamples::e1;
            RasterizationSamples msaaSupportDepthStencilTarget = RasterizationSamples::e1;
            RasterizationSamples msaaSupportShaderResourceView = RasterizationSamples::e1;
            RasterizationSamples msaaSupportUnorderedAccessView = RasterizationSamples::e1;

            // --- Memory / Alignment ---
            u32 bufferImageRowAlignment = 0;         ///< D3D12_TEXTURE_DATA_PITCH_ALIGNMENT / VkPhysicalDeviceLimits::optimalBufferCopyRowPitchAlignment
            u32 bufferImageCopyOffsetAlignment = 0;  ///< D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT / VkPhysicalDeviceLimits::optimalBufferCopyOffsetAlignment
            u32 minUniformBufferOffsetAlignment = 0; ///< D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT / Vulkan equivalent
            u32 minStorageBufferOffsetAlignment = 0; ///< SRV or UAV buffer

            // --- Queue capabilities ---
            u32 graphicsQueueCount = 0;
            u32 computeQueueCount = 0;
            u32 transferQueueCount = 0;
            bool bHasDedicatedComputeQueue = false;
            bool bHasDedicatedTransferQueue = false;

            // --- Resource limits ---
            u32 maxTextureWidth = 0;
            u32 maxTextureHeight = 0;
            u32 maxTextureDepth = 0;
            u32 maxTextureArrayLayers = 0;
            u32 maxSamplerAnisotropy = 0;

            f32 minLineWidth = 0.0f;
            f32 maxLineWidth = 0.0f;

            PYRO_NODISCARD bool operator==(const DevicePropertiesInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DevicePropertiesInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Tracks live GPU and host resource statistics for the device.
         * Useful for diagnostics, debugging, and memory leak detection.
         */
        struct DeviceStatusInfo {
            // --- Host (CPU) memory usage ---
            usize numHostAllocations = 0;    ///< Total number of allocations made by CPU-side pools.
            usize numHostAllocatedBytes = 0; ///< Total bytes allocated in host (system) memory.

            // --- GPU (device) memory usage ---
            DeviceSize numDeviceMemoryAllocations = 0; ///< Number of GPU memory allocations (heaps / VkDeviceMemory).
            DeviceSize numDeviceAllocatedBytes = 0;    ///< Approximate total GPU memory allocated.

            // --- Resource creation counters ---
            u32 numBufferResourcesCreated = 0; ///< Total GPU buffers created.
            u32 numImageResourcesCreated = 0;  ///< Total GPU images/textures created.
            u32 numPipelineObjectsCreated = 0; ///< Total graphics/compute pipelines created.
            u32 numDescriptorHeapsCreated = 0; ///< Descriptor heaps / pools created.
            u32 numCommandBuffersCreated = 0;  ///< Command buffers or command lists created.

            // --- Active / live resources ---
            u32 numBuffersAlive = 0;
            u32 numImagesAlive = 0;
            u32 numPipelinesAlive = 0;
            u32 numSamplersAlive = 0;

            // --- Frame statistics ---
            u64 numQueueSubmits = 0;

            // --- Optional GPU memory fragmentation / budget info ---
            DeviceSize availableVideoMemory = 0; ///< Remaining VRAM at query time.

            PYRO_NODISCARD bool operator==(const DeviceStatusInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DeviceStatusInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * Parameters for fine-grained synchronisation of a semaphore signal/wait.
         */
        struct SemaphoreSubmitInfo {
            Semaphore semaphore = nullptr;                          /**< Semaphore handle*/
            PipelineStageFlags stage = PipelineStageFlagBits::NONE; /**< Pipeline stage where the signal/wait should occur*/

            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const SemaphoreSubmitInfo&) const = default;
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const SemaphoreSubmitInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * Parameters for signalling a fence with a value.
         */
        struct FenceSubmitInfo {
            IFence* fence = nullptr; /**< Fence handle*/
            u64 value = 0;           /**< Fence value that should be signalled */

            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const FenceSubmitInfo&) const = default;
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const FenceSubmitInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * Parameters for submitting a command queue.
         */
        struct CommandQueueSubmitInfo {
            /**
             * @brief the command queue to submit. *MUST* be non-null;
             */
            ICommandQueue* queue = nullptr;
            /**
             * @brief Semaphores to wait on, if applicable
             * This is required for multiple command queue synchronisation, e.g.
             * CommandQueueA submit:
             *      - Signal: [(Semaphore0, COMPUTE_SHADER)]
             *      - Wait: []
             * CommandQueueB submit:
             *      - Signal: []
             *      - Wait: [(Semaphore0, COMPUTE_SHADER)]
             */
            eastl::span<const SemaphoreSubmitInfo> waitSemaphores = {};
            /**
             * @brief Semaphores to signal, if applicable
             */
            eastl::span<const SemaphoreSubmitInfo> signalSemaphores = {};
            /**
             * @brief Semaphores to signal that the swapchain can present the rendered image, if applicable
             * @note: this is *NOT* for swapchain acquiring the next available image, but rather a signal from
             * the queue to the presenting engine that the frame is ready to be presented. This is to cope with
             * the seperation of frames in flight and swap chain buffer count.
             */
            eastl::span<const SemaphoreSubmitInfo> signalPresentReadySemaphores = {};
            /**
             * @brief Pairs of fences to signal, with a value.
             */
            eastl::span<const FenceSubmitInfo> signalFences = {};

            PYRO_NODISCARD bool operator==(const CommandQueueSubmitInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CommandQueueSubmitInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * Parameters for presenting the swap chains inside of a command queue.
         */
        struct CommandQueuePresentInfo {
            /**
             * @brief the command queue to present. *MUST* be non-null;
             */
            ICommandQueue* queue = nullptr;
            /**
             * @brief Semaphores that the present operation must wait on, if applicable
             */
            eastl::span<Semaphore> waitSemaphores = {};

            PYRO_NODISCARD bool operator==(const CommandQueuePresentInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CommandQueuePresentInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Interface for GPU device operations and resource management.
         *
         * Provides methods for creating, querying, and destroying GPU resources,
         * as well as for submitting commands and synchronising workloads.
         */
        struct IDevice {
            IDevice() = default;
            virtual ~IDevice() = default;

            // ---------------------------------------------------------------------
            // Validation
            // ---------------------------------------------------------------------

            /**
             * @brief Check whether a memory block handle is valid.
             */
            PYRO_NODISCARD virtual bool IsMemoryBlockValid(MemoryBlock handle) const = 0;

            /**
             * @brief Check whether a buffer handle is valid.
             */
            PYRO_NODISCARD virtual bool IsBufferValid(Buffer handle) const = 0;

            /**
             * @brief Check whether an image handle is valid.
             */
            PYRO_NODISCARD virtual bool IsImageValid(Image handle) const = 0;

            /**
             * @brief Check whether a shader resource handle is valid.
             */
            PYRO_NODISCARD virtual bool IsShaderResourceValid(ShaderResourceId id) const = 0;

            /**
             * @brief Check whether an unordered access view (UAV) handle is valid.
             */
            PYRO_NODISCARD virtual bool IsUnorderedAccessValid(UnorderedAccessId id) const = 0;

            /**
             * @brief Check whether a sampler handle is valid.
             */
            PYRO_NODISCARD virtual bool IsSamplerValid(SamplerId id) const = 0;

            // Convenience overloads
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(MemoryBlock handle) const { return IsMemoryBlockValid(handle); }
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(Buffer handle) const { return IsBufferValid(handle); }
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(Image handle) const { return IsImageValid(handle); }
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(ShaderResourceId id) const { return IsShaderResourceValid(id); }
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(UnorderedAccessId id) const { return IsUnorderedAccessValid(id); }
            PYRO_NODISCARD PYRO_FORCEINLINE bool IsValid(SamplerId id) const { return IsSamplerValid(id); }

            // ---------------------------------------------------------------------
            // Resource Info Queries
            // ---------------------------------------------------------------------

            /**
             * @brief Retrieves memory block description.
             */
            PYRO_NODISCARD virtual const MemoryBlockInfo& GetMemoryBlockInfo(MemoryBlock memory) const = 0;

            /**
             * @brief Retrieves buffer description.
             */
            PYRO_NODISCARD virtual const BufferInfo& GetBufferInfo(Buffer buffer) const = 0;

            /**
             * @brief Retrieves image description.
             */
            PYRO_NODISCARD virtual const ImageInfo& GetImageInfo(Image image) const = 0;

            /**
             * @brief Retrieves shader resource view description.
             */
            PYRO_NODISCARD virtual const GpuResourceInfo& GetShaderResourceInfo(ShaderResourceId id) const = 0;

            /**
             * @brief Retrieves unordered access view description.
             */
            PYRO_NODISCARD virtual const GpuResourceInfo& GetUnorderedAccessInfo(UnorderedAccessId id) const = 0;

            /**
             * @brief Retrieves sampler description.
             */
            PYRO_NODISCARD virtual const SamplerInfo& GetSamplerInfo(SamplerId id) const = 0;

            /**
             * @brief Retrieves render target description.
             */
            PYRO_NODISCARD virtual const RenderTargetInfo& GetRenderTargetInfo(RenderTarget renderTarget) const = 0;

            /**
             * @brief Retrieves raster pipeline object description.
             */
            PYRO_NODISCARD virtual const RasterPipelineInfo& GetRasterPipelineInfo(RasterPipeline pipeline) const = 0;

            /**
             * @brief Retrieves compute pipeline object description.
             */
            PYRO_NODISCARD virtual const ComputePipelineInfo& GetComputePipelineInfo(ComputePipeline pipeline) const = 0;

            /**
             * @brief Retrieves semaphore description.
             */
            PYRO_NODISCARD virtual const SemaphoreInfo& GetSemaphoreInfo(Semaphore semaphore) const = 0;

            /**
             * @brief Retrieves BLAS description.
             */
            PYRO_NODISCARD virtual const BlasInfo& GetBlasInfo(BlasId blas) const = 0;

            /**
             * @brief Retrieves TLAS description.
             */
            PYRO_NODISCARD virtual const TlasInfo& GetTlasInfo(TlasId tlas) const = 0;


            // ---------------------------------------------------------------------
            // Memory Access
            // ---------------------------------------------------------------------

            /**
             * @brief - REQUIRES BDA SUPPORT -
             * Returns the device (GPU) address of a buffer, if supported.
             * This address may or may not be a valid address to use inside a shader, usage varies by API.
             */
            PYRO_NODISCARD virtual DeviceAddress BufferDeviceAddress(Buffer buffer) const = 0;


            /**
             * @brief Returns the host (CPU) mapped pointer for a buffer. This will not be a valid
             * pointer if the buffer is not host visible.
             */
            PYRO_NODISCARD virtual u8* BufferHostAddress(Buffer buffer) const = 0;

            /**
             * @brief Returns the host-mapped pointer for a buffer, cast to the requested type.
             */
            template <typename T>
            PYRO_NODISCARD PYRO_FORCEINLINE T* BufferHostAddressAs(Buffer buffer) const {
                return reinterpret_cast<T*>(BufferHostAddress(buffer));
            }

            // ---------------------------------------------------------------------
            // Memory requirements
            // ---------------------------------------------------------------------

            /**
             * @brief Returns the size requirements (in bytes) for the entire image resource.
             *
             * This is the total memory footprint needed to store the image including all mip levels,
             * array layers, and alignment constraints required by the GPU.
             */
            PYRO_NODISCARD virtual DeviceSize ImageSizeRequirements(Image image) const = 0;

            /**
             * @brief Returns the required row pitch (stride in bytes) for a specific subresource of the image.
             *
             * The row pitch is the number of bytes the GPU expects between consecutive rows in a buffer
             * when copying to or from this image subresource. It must be used when performing
             * buffer-to-image or image-to-buffer copies to avoid out-of-bounds errors.
             *
             * Row width is the minimal width that needs to be queried **INCLUDING** the format size. For a buffer-image copy, this is the extent of your copy region.
             */
            PYRO_NODISCARD virtual u32 ImageSubresourceRowPitch(Image image, u32 rowWidth, ImageSlice slice = {}) const = 0;

            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             *
             * Returns the size requirements for build, scratch and update buffers for a BLAS
             */
            PYRO_NODISCARD virtual AccelerationStructureBuildSizesInfo BlasSizeRequirements(const BlasBuildInfo& info) const = 0;
            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             *
             * Returns the size requirements for build, scratch and update buffers for a TLAS
             */
            PYRO_NODISCARD virtual AccelerationStructureBuildSizesInfo TlasSizeRequirements(const TlasBuildInfo& info) const = 0;

            // ---------------------------------------------------------------------
            // Resource Creation
            // ---------------------------------------------------------------------

            /**
             * @brief Creates a block of device memory with the specified parameters
             * This is to be used for virtual suballocations. This is significantly more efficient than
             * creating a buffer without a specified block.
             */
            PYRO_NODISCARD virtual MemoryBlock CreateMemoryBlock(const MemoryBlockInfo& info) = 0;
            /**
             * @brief Creates a buffer with the specified parameters
             * @note If the `memoryBlock` is a valid handle, and the virtual allocation failed, then `PYRO_NULL_BUFFER` will be returned
             * This is *NOT* an error, and simply means that memory block ran out of space.
             */
            PYRO_NODISCARD virtual Buffer CreateBuffer(const BufferInfo& info) = 0;
            /**
             * @brief Creates an image with the specified parameters
             * @note If the `memoryBlock` is a valid handle, and the virtual allocation failed, then `PYRO_NULL_IMAGE` will be returned
             * This is *NOT* an error, and simply means that memory block ran out of space.
             */
            PYRO_NODISCARD virtual Image CreateImage(const ImageInfo& info) = 0;
            /**
             * @brief Creates a shader resource view with the specified parameters. ShaderResourceId::index is an index into
             * the descriptor heap that can be used to index into a bindless heap in a shader.
             */
            PYRO_NODISCARD virtual ShaderResourceId CreateShaderResource(const GpuResourceInfo& info) = 0;
            /**
             * @brief Creates an unordered access view with the specified parameters. This handle must be bound
             * and cannot be used for bindless shader indexing.
             */
            PYRO_NODISCARD virtual UnorderedAccessId CreateUnorderedAccess(const GpuResourceInfo& info) = 0;
            /**
             * @brief Creates a sampler with the specified parameters. SamplerId::index is an index into
             * the descriptor heap that can be used to index into a bindless heap in a shader.
             */
            PYRO_NODISCARD virtual SamplerId CreateSampler(const SamplerInfo& info) = 0;

            /**
             * @brief Creates a render target view (either a colour target or depth-stencil) with the specified parameters
             */
            PYRO_NODISCARD virtual RenderTarget CreateRenderTarget(const RenderTargetInfo& info) = 0;
            /**
             * @brief Creates a raster pipeline with the specified parameters
             */
            PYRO_NODISCARD virtual RasterPipeline CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) = 0;
            /**
             * @brief Creates a compute pipeline with the specified parameters
             */
            PYRO_NODISCARD virtual ComputePipeline CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) = 0;
            /**
             * @brief Creates a swap chain with the specified parameters
             */
            PYRO_NODISCARD virtual ISwapChain* CreateSwapChain(const SwapChainInfo& info) = 0;
            /**
             * @brief Creates a semaphore with the specified parameters
             */
            PYRO_NODISCARD virtual Semaphore CreateSemaphore(const SemaphoreInfo& info) = 0;
            /**
             * @brief Creates a fence with the specified parameters
             */
            PYRO_NODISCARD virtual IFence* CreateFence(const FenceInfo& info) = 0;
            /**
             * @brief Creates a query pool for GPU command timestamps
             */
            PYRO_NODISCARD virtual ITimestampQueryPool* CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) = 0;

            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             * Creates a bottom level acceleration structure to be referenced by a Tlas.
             */
            PYRO_NODISCARD virtual BlasId CreateBlas(const BlasInfo& info) = 0;
            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             * Creates a top level acceleration structure with the given parameters. This returns a handle
             * that is meant to be passed to a shader, to index into a runtime array of Tlas descriptors
             */
            PYRO_NODISCARD virtual TlasId CreateTlas(const TlasInfo& info) = 0;


            // ---------------------------------------------------------------------
            // Resource Destruction
            // ---------------------------------------------------------------------

            /**
             * @brief Destroys the memory block.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             * @note Make sure all resources making use of this memory handle have been destroyed prior to this!
             */
            virtual void DestroyMemoryBlock(MemoryBlock& memory, bool bDefer = false) = 0;
            /**
             * @brief Destroys the buffer.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyBuffer(Buffer& buffer, bool bDefer = false) = 0;
            /**
             * @brief Destroys the image.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyImage(Image& image, bool bDefer = false) = 0;
            /**
             * @brief Destroys the shader resource view.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyShaderResource(ShaderResourceId& srv, bool bDefer = false) = 0;
            /**
             * @brief Destroys the unordered access view.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyUnorderedAccess(UnorderedAccessId& uav, bool bDefer = false) = 0;
            /**
             * @brief Destroys the sampler object.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroySampler(SamplerId& sampler, bool bDefer = false) = 0;

            /**
             * @brief Destroys the render target.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyRenderTarget(RenderTarget& renderTarget, bool bDefer = false) = 0;
            /**
             * @brief Destroys the pipeline.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyRasterPipeline(RasterPipeline& pipeline, bool bDefer = false) = 0;
            /**
             * @brief Destroys the pipeline.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyComputePipeline(ComputePipeline& pipeline, bool bDefer = false) = 0;
            /**
             * @brief Destroys the swap chain.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroySwapChain(ISwapChain*& swapChain, bool bDefer = false) = 0;
            /**
             * @brief Destroys the semaphore.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroySemaphore(Semaphore& semaphore, bool bDefer = false) = 0;
            /**
             * @brief Destroys the fence.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyFence(IFence*& fence, bool bDefer = false) = 0;
            /**
             * @brief Destroys the query pool.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool, bool bDefer = false) = 0;
            
            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             * @brief Destroys the BLAS.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyBlas(BlasId& blas, bool bDefer = false) = 0;
            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             * @brief Destroys the TLAS.
             * When `bDefer` is false, the object is destroyed immediately, and the handle is set to NULL.
             * If `bDefer` is true, the object will be scheduled to be destroyed after all queue submits have completed, and `CollectGarbage()` is required to be called.
             */
            virtual void DestroyTlas(TlasId& tlas, bool bDefer = false) = 0;

            // Convenience overloads

            PYRO_FORCEINLINE void Destroy(MemoryBlock& memory, bool bDefer = false) { DestroyMemoryBlock(memory, bDefer); }
            PYRO_FORCEINLINE void Destroy(Buffer& buffer, bool bDefer = false) { DestroyBuffer(buffer, bDefer); }
            PYRO_FORCEINLINE void Destroy(Image& image, bool bDefer = false) { DestroyImage(image, bDefer); }
            PYRO_FORCEINLINE void Destroy(ShaderResourceId& srv, bool bDefer = false) { DestroyShaderResource(srv, bDefer); }
            PYRO_FORCEINLINE void Destroy(UnorderedAccessId& uav, bool bDefer = false) { DestroyUnorderedAccess(uav, bDefer); }
            PYRO_FORCEINLINE void Destroy(SamplerId& sampler, bool bDefer = false) { DestroySampler(sampler, bDefer); }
            PYRO_FORCEINLINE void Destroy(RasterPipeline& pipeline, bool bDefer = false) { DestroyRasterPipeline(pipeline, bDefer); }
            PYRO_FORCEINLINE void Destroy(ComputePipeline& pipeline, bool bDefer = false) { DestroyComputePipeline(pipeline, bDefer); }
            PYRO_FORCEINLINE void Destroy(ISwapChain*& swapChain, bool bDefer = false) { DestroySwapChain(swapChain, bDefer); }
            PYRO_FORCEINLINE void Destroy(RenderTarget& renderTarget, bool bDefer = false) { DestroyRenderTarget(renderTarget, bDefer); }
            PYRO_FORCEINLINE void Destroy(Semaphore& semaphore, bool bDefer = false) { DestroySemaphore(semaphore, bDefer); }
            PYRO_FORCEINLINE void Destroy(IFence*& fence, bool bDefer = false) { DestroyFence(fence, bDefer); }
            PYRO_FORCEINLINE void Destroy(ITimestampQueryPool*& queryPool, bool bDefer = false) { DestroyTimestampQueryPool(queryPool, bDefer); }
            PYRO_FORCEINLINE void Destroy(BlasId& blas, bool bDefer = false) { DestroyBlas(blas, bDefer); }
            PYRO_FORCEINLINE void Destroy(TlasId& tlas, bool bDefer = false) { DestroyTlas(tlas, bDefer); }

            PYRO_FORCEINLINE void DestroyImmediately(MemoryBlock memory) { DestroyMemoryBlock(memory, false); }
            PYRO_FORCEINLINE void DestroyImmediately(Buffer buffer) { DestroyBuffer(buffer, false); }
            PYRO_FORCEINLINE void DestroyImmediately(Image image) { DestroyImage(image, false); }
            PYRO_FORCEINLINE void DestroyImmediately(ShaderResourceId srv) { DestroyShaderResource(srv, false); }
            PYRO_FORCEINLINE void DestroyImmediately(UnorderedAccessId uav) { DestroyUnorderedAccess(uav, false); }
            PYRO_FORCEINLINE void DestroyImmediately(SamplerId sampler) { DestroySampler(sampler, false); }
            PYRO_FORCEINLINE void DestroyImmediately(RasterPipeline pipeline) { DestroyRasterPipeline(pipeline, false); }
            PYRO_FORCEINLINE void DestroyImmediately(ComputePipeline pipeline) { DestroyComputePipeline(pipeline, false); }
            PYRO_FORCEINLINE void DestroyImmediately(ISwapChain* swapChain) { DestroySwapChain(swapChain, false); }
            PYRO_FORCEINLINE void DestroyImmediately(RenderTarget renderTarget) { DestroyRenderTarget(renderTarget, false); }
            PYRO_FORCEINLINE void DestroyImmediately(Semaphore semaphore) { DestroySemaphore(semaphore, false); }
            PYRO_FORCEINLINE void DestroyImmediately(IFence* fence) { DestroyFence(fence, false); }
            PYRO_FORCEINLINE void DestroyImmediately(ITimestampQueryPool* queryPool) { DestroyTimestampQueryPool(queryPool, false); }
            PYRO_FORCEINLINE void DestroyImmediately(BlasId blas) { DestroyBlas(blas, false); }
            PYRO_FORCEINLINE void DestroyImmediately(TlasId tlas) { DestroyTlas(tlas, false); }

            PYRO_FORCEINLINE void DestroyDeferred(MemoryBlock memory) { DestroyMemoryBlock(memory, true); }
            PYRO_FORCEINLINE void DestroyDeferred(Buffer buffer) { DestroyBuffer(buffer, true); }
            PYRO_FORCEINLINE void DestroyDeferred(Image image) { DestroyImage(image, true); }
            PYRO_FORCEINLINE void DestroyDeferred(ShaderResourceId srv) { DestroyShaderResource(srv, true); }
            PYRO_FORCEINLINE void DestroyDeferred(UnorderedAccessId uav) { DestroyUnorderedAccess(uav, true); }
            PYRO_FORCEINLINE void DestroyDeferred(SamplerId sampler) { DestroySampler(sampler, true); }
            PYRO_FORCEINLINE void DestroyDeferred(RasterPipeline pipeline) { DestroyRasterPipeline(pipeline, true); }
            PYRO_FORCEINLINE void DestroyDeferred(ComputePipeline pipeline) { DestroyComputePipeline(pipeline, true); }
            PYRO_FORCEINLINE void DestroyDeferred(ISwapChain* swapChain) { DestroySwapChain(swapChain, true); }
            PYRO_FORCEINLINE void DestroyDeferred(RenderTarget renderTarget) { DestroyRenderTarget(renderTarget, true); }
            PYRO_FORCEINLINE void DestroyDeferred(Semaphore semaphore) { DestroySemaphore(semaphore, true); }
            PYRO_FORCEINLINE void DestroyDeferred(IFence* fence) { DestroyFence(fence, true); }
            PYRO_FORCEINLINE void DestroyDeferred(ITimestampQueryPool* queryPool) { DestroyTimestampQueryPool(queryPool, true); }
            PYRO_FORCEINLINE void DestroyDeferred(BlasId blas) { DestroyBlas(blas, true); }
            PYRO_FORCEINLINE void DestroyDeferred(TlasId tlas) { DestroyTlas(tlas, true); }

            // ---------------------------------------------------------------------
            // Support Queries
            // ---------------------------------------------------------------------

            /**
             * @brief Selects the first supported format from a list of candidates.
             */
            PYRO_NODISCARD virtual eastl::optional<Format> PickSupportedFormat(
                const eastl::span<Format>& candidates, FormatFeatureFlags features) const = 0;

            /**
             * @brief Retrieves all available command queues.
             */
            PYRO_NODISCARD virtual eastl::span<ICommandQueue*> GetCommandQueues() = 0;

            /**
             * @brief Retrieves the present (display) command queue.
             */
            PYRO_NODISCARD virtual ICommandQueue* GetPresentQueue() = 0;

            // ---------------------------------------------------------------------
            // Submission & Synchronization
            // ---------------------------------------------------------------------

            /**
             * @brief Destroys all pending objects for deletion. This is required to be called after a deferred destruction call, otherwise resources will be accumulated.
             * This should ONLY be called AFTER all queues using the resources have been submitted, or the device may assume the resource was never used, and prematurely destroy them.
             * The following is an example of CORRECT usage:
             *
             * ======================================================
             *
             * -- Frame 1
             *
             *   - Destroy resources A
             *
             *   - Use resources A
             *
             *   - Submit Queue 1
             *
             *   - Use resources A
             *
             *   - Submit Queue 2
             *
             *   - Collect Garbage
             *
             * -- Frame 2
             *
             *   - Destroy resources B
             *
             *   - Use resources B
             *
             *   - Collect Garbage
             *
             *   - Submit Queue 1
             *
             *   - Destroy resources C
             *
             *   - Use resources C
             *
             *   - Submit Queue 2
             *
             *   - Collect Garbage
             *
             * ======================================================
             *
             * @note this is automatically called upon device destruction.
             */
            virtual void CollectGarbage() = 0;

            /**
             * @brief Blocks until the device has finished all queued work.
             */
            virtual void WaitIdle() = 0;

            /**
             * @brief Submits work to a command queue.
             */
            virtual void SubmitQueue(const CommandQueueSubmitInfo& info) = 0;

            /**
             * @brief Presents an image to the swapchain using the present queue.
             */
            virtual void PresentQueue(const CommandQueuePresentInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Device Properties
            // ---------------------------------------------------------------------

            /**
             * @brief Returns general device properties.
             */
            PYRO_NODISCARD virtual const DeviceInfo& Info() const = 0;
            /**
             * @brief Returns hardware limits and capabilities.
             */
            PYRO_NODISCARD virtual const DevicePropertiesInfo& Properties() const = 0;
            /**
             * @brief Returns hardware features.
             */
            PYRO_NODISCARD virtual const DeviceFeaturesInfo& Features() const = 0;
            /**
             * @brief Returns the real time hardware status.
             */
            PYRO_NODISCARD virtual DeviceStatusInfo Status() const = 0;

            // Convenience create overloads
            PYRO_NODISCARD PYRO_FORCEINLINE MemoryBlock Create(const MemoryBlockInfo& info) { return CreateMemoryBlock(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE Buffer Create(const BufferInfo& info) { return CreateBuffer(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE Image Create(const ImageInfo& info) { return CreateImage(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE SamplerId Create(const SamplerInfo& info) { return CreateSampler(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE RasterPipeline Create(const RasterPipelineInfo& info, const RasterPipelineShaderStages& stages) { return CreateRasterPipeline(info, stages); }
            PYRO_NODISCARD PYRO_FORCEINLINE ComputePipeline Create(const ComputePipelineInfo& info, const ShaderInfo& shader) { return CreateComputePipeline(info, shader); }
            PYRO_NODISCARD PYRO_FORCEINLINE ISwapChain* Create(const SwapChainInfo& info) { return CreateSwapChain(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE RenderTarget Create(const RenderTargetInfo& info) { return CreateRenderTarget(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE Semaphore Create(const SemaphoreInfo& info) { return CreateSemaphore(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE IFence* Create(const FenceInfo& info) { return CreateFence(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE ITimestampQueryPool* Create(const TimestampQueryPoolInfo& info) { return CreateTimestampQueryPool(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE BlasId Create(const BlasInfo& info) { return CreateBlas(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE TlasId Create(const TlasInfo& info) { return CreateTlas(info); }
        };
    } // namespace RHI
} // namespace PyroshockStudios