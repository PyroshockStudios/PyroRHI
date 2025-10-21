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
        // TODO: maybe it's better to refactor this and split it into DeviceStatistics or something?
        struct DeviceInfo {
            usize numAllocations = {};
            usize numAllocatedBytes = {};
            eastl::string name = {};
            eastl::string vendor = {};
            u32 driverVersion = {};

            PYRO_NODISCARD bool operator==(const DeviceInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DeviceInfo&) const = default;
        };

        /**
         * @brief Describes the properties of the device.
         */
        struct DevicePropertiesInfo {
            // TODO, refactor this into a bit mask?
            RasterizationSamples maxRenderTargetSamples = RasterizationSamples::e1;
            RasterizationSamples maxShaderResourceImageSamples = RasterizationSamples::e1;
            u32 bufferImageRowAlignment = 0;
            u32 bufferImageCopyOffsetAlignment = 0;
            bool bSupportsHeadlessSwapChainWindow = false;
            bool bSupportsRayTracing = false;
            
            PYRO_NODISCARD bool operator==(const DevicePropertiesInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DevicePropertiesInfo&) const = default;
        };

        /**
         * Parameters for fine-grained synchronisation of a semaphore signal/wait.
         */
        struct SemaphoreSubmitInfo {
            Semaphore semaphore = nullptr;                          /**< Semaphore handle*/
            PipelineStageFlags stage = PipelineStageFlagBits::NONE; /**< Pipeline stage where the signal/wait should occur*/

            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const SemaphoreSubmitInfo&) const = default;
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const SemaphoreSubmitInfo&) const = default;
        };

        /**
         * Parameters for signalling a fence with a value.
         */
        struct FenceSubmitInfo {
            IFence* fence = nullptr; /**< Fence handle*/
            u64 value = 0;           /**< Fence value that should be signalled */

            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const FenceSubmitInfo&) const = default;
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const FenceSubmitInfo&) const = default;
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
            PYRO_NODISCARD virtual const GPUResourceInfo& GetShaderResourceInfo(ShaderResourceId id) const = 0;

            /**
             * @brief Retrieves unordered access view description.
             */
            PYRO_NODISCARD virtual const GPUResourceInfo& GetUnorderedAccessInfo(UnorderedAccessId id) const = 0;

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
            PYRO_NODISCARD virtual const BLASInfo& GetBLASInfo(BLAS blas) const = 0;

            /**
             * @brief Retrieves TLAS description.
             */
            PYRO_NODISCARD virtual const TLASInfo& GetTLASInfo(TLASId tlas) const = 0;


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
            // Resource Queries
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
            PYRO_NODISCARD virtual u32 ImageSubresourceRowPitch(Image image, ImageSlice slice, u32 rowWidth) const = 0;

            /**
             * @brief Returns the minimum required size for building a bottom-level acceleration structure  
             */
            PYRO_NODISCARD virtual DeviceSize BLASBuildSizeRequirements(BLAS blas, eastl::span<const BLASGeometryInfo>, u32 primitiveCount) const = 0;
            /**
             * @brief Returns the minimum required size for building a top-level acceleration structure  
             */
            PYRO_NODISCARD virtual DeviceSize TLASBuildSizeRequirements(TLASId tlas, eastl::span<const TLASInstanceInfo>, u32 instanceCount) const = 0;


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
            PYRO_NODISCARD virtual ShaderResourceId CreateShaderResource(const GPUResourceInfo& info) = 0;
            /**
             * @brief Creates an unordered access view with the specified parameters. This handle must be bound
             * and cannot be used for bindless shader indexing.
             */
            PYRO_NODISCARD virtual UnorderedAccessId CreateUnorderedAccess(const GPUResourceInfo& info) = 0;
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
             * Creates a bottom level acceleration structure to be referenced by a TLAS.
             */
            PYRO_NODISCARD virtual BLAS CreateBLAS(const BLASInfo& info) = 0;
            /**
             * @brief - REQUIRES RAY TRACING SUPPORT -
             * Creates a top level acceleration structure with the given parameters. This returns a handle
             * that is meant to be passed to a shader, to index into a runtime array of TLAS descriptors
             */
            PYRO_NODISCARD virtual TLASId CreateTLAS(const TLASInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Resource Destruction
            // ---------------------------------------------------------------------

            /**
             * @brief Immediately destroys the memory block, and sets the handle to NULL
             * @note Make sure all resources making use of this memory handle have been destroyed prior to this!
             */
            virtual void DestroyMemoryBlock(MemoryBlock& memory) = 0;
            /**
             * @brief Immediately destroys the buffer, and sets the handle to NULL
             */
            virtual void DestroyBuffer(Buffer& buffer) = 0;
            /**
             * @brief Immediately destroys the image, and sets the handle to NULL
             */
            virtual void DestroyImage(Image& image) = 0;
            /**
             * @brief Immediately destroys the shader resource view, and sets the handle to NULL
             */
            virtual void DestroyShaderResource(ShaderResourceId& srv) = 0;
            /**
             * @brief Immediately destroys the unordered access view, and sets the handle to NULL
             */
            virtual void DestroyUnorderedAccess(UnorderedAccessId& uav) = 0;
            /**
             * @brief Immediately destroys the sampler, and sets the handle to NULL
             */
            virtual void DestroySampler(SamplerId& sampler) = 0;
            /**
             * @brief Immediately destroys the render target, and sets the handle to NULL
             */
            virtual void DestroyRenderTarget(RenderTarget& renderTarget) = 0;
            /**
             * @brief Immediately destroys the raster pipeline, and sets the handle to NULL
             */
            virtual void DestroyRasterPipeline(RasterPipeline& pipeline) = 0;
            /**
             * @brief Immediately destroys the compute pipeline, and sets the handle to NULL
             */
            virtual void DestroyComputePipeline(ComputePipeline& pipeline) = 0;
            /**
             * @brief Immediately destroys the swap chain, and sets the handle to NULL
             */
            virtual void DestroySwapChain(ISwapChain*& swapChain) = 0;
            /**
             * @brief Immediately destroys the semaphore, and sets the handle to NULL
             */
            virtual void DestroySemaphore(Semaphore& semaphore) = 0;
            /**
             * @brief Immediately destroys the fence, and sets the handle to NULL
             */
            virtual void DestroyFence(IFence*& fence) = 0;
            /**
             * @brief Immediately destroys the query pool, and sets the handle to NULL
             */
            virtual void DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool) = 0;
           
            /**
             * @brief Immediately destroys the BLAS, and sets the handle to NULL
             */
            virtual void DestroyBLAS(BLAS& blas) = 0;
            /**
             * @brief Immediately destroys the TLAS, and sets the handle to NULL
             */
            virtual void DestroyTLAS(TLASId& tlas) = 0;

            // Convenience overloads
            PYRO_FORCEINLINE void Destroy(MemoryBlock& memory) { DestroyMemoryBlock(memory); }
            PYRO_FORCEINLINE void Destroy(Buffer& buffer) { DestroyBuffer(buffer); }
            PYRO_FORCEINLINE void Destroy(Image& image) { DestroyImage(image); }
            PYRO_FORCEINLINE void Destroy(ShaderResourceId& srv) { DestroyShaderResource(srv); }
            PYRO_FORCEINLINE void Destroy(UnorderedAccessId& uav) { DestroyUnorderedAccess(uav); }
            PYRO_FORCEINLINE void Destroy(SamplerId& sampler) { DestroySampler(sampler); }
            PYRO_FORCEINLINE void Destroy(RasterPipeline& pipeline) { DestroyRasterPipeline(pipeline); }
            PYRO_FORCEINLINE void Destroy(ComputePipeline& pipeline) { DestroyComputePipeline(pipeline); }
            PYRO_FORCEINLINE void Destroy(ISwapChain*& swapChain) { DestroySwapChain(swapChain); }
            PYRO_FORCEINLINE void Destroy(RenderTarget& renderTarget) { DestroyRenderTarget(renderTarget); }
            PYRO_FORCEINLINE void Destroy(Semaphore& semaphore) { DestroySemaphore(semaphore); }
            PYRO_FORCEINLINE void Destroy(IFence*& fence) { DestroyFence(fence); }
            PYRO_FORCEINLINE void Destroy(ITimestampQueryPool*& queryPool) { DestroyTimestampQueryPool(queryPool); }
           
            PYRO_FORCEINLINE void Destroy(BLAS& blas) { DestroyBLAS(blas); }
            PYRO_FORCEINLINE void Destroy(TLASId& tlas) { DestroyTLAS(tlas); }

            // ---------------------------------------------------------------------
            // Support Queries
            // ---------------------------------------------------------------------

            /**
             * @brief Selects the first supported format from a list of candidates.
             */
            PYRO_NODISCARD virtual eastl::optional<Format> PickSupportedFormat(
                const eastl::span<Format>& candidates,
                FormatFeatureFlags features) = 0;

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
            PYRO_NODISCARD virtual const DeviceInfo& GetInfo() = 0;

            /**
             * @brief Returns hardware limits and capabilities.
             */
            PYRO_NODISCARD virtual const DevicePropertiesInfo& GetProperties() = 0;

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
            PYRO_NODISCARD PYRO_FORCEINLINE BLAS Create(const BLASInfo& info) { return CreateBLAS(info); }
            PYRO_NODISCARD PYRO_FORCEINLINE TLASId Create(const TLASInfo& info) { return CreateTLAS(info); }
        };

    } // namespace RHI
} // namespace PyroshockStudios