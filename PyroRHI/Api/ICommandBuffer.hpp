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

#include <EASTL/array.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/optional.h>
#include <EASTL/string.h>
#include <EASTL/variant.h>

#include <PyroCommon/Concepts.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <PyroRHI/Api/Limits.hpp>
#include <PyroRHI/Api/Pipeline.hpp>
#include <PyroRHI/Api/RenderTarget.hpp>
#include <PyroRHI/Api/Types.hpp>
#include <PyroRHI/Api/AccelerationStructure.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        /**
         * @brief Parameters for getting a command buffer.
         */
        struct CommandBufferInfo {
            /**
             * @brief Optional name for debugging/profiling.
             * */
            eastl::string name = {};
            PYRO_NODISCARD bool operator==(const CommandBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CommandBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };
        /**
         * @brief Parameters for copying a buffer to a buffer.
         */
        struct CopyBufferToBufferInfo {
            /**
             * @brief Source buffer handle. *MUST* be non-null
             * */
            Buffer srcBuffer = PYRO_NULL_BUFFER;
            /**
             * @brief Destination buffer handle. *MUST* be non-null
             * */
            Buffer dstBuffer = PYRO_NULL_BUFFER;
            /**
             * @brief Source buffer copy offset.
             * */
            DeviceSize srcOffset = {};
            /**
             * @brief Destination buffer copy offset.
             * */
            DeviceSize dstOffset = {};
            /**
             * @brief Copy size. dstOffset + size must not exceed the size of the destination buffer,
             * and neither should srcOffset + size exceed the size of the source buffer.
             * */
            DeviceSize size = {};

            PYRO_NODISCARD bool operator==(const CopyBufferToBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CopyBufferToBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };
        /**
         * @brief Parameters for copying a buffer to an image.
         */
        struct CopyBufferToImageInfo {
            /**
             * @brief Source buffer handle. *MUST* be non-null.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Source buffer copy offset in bytes.
             */
            DeviceSize bufferOffset = {};
            /**
             * @brief Destination image handle. *MUST* be non-null.
             */
            Image image = PYRO_NULL_IMAGE;
            /**
             * @brief Target image slice to copy into.
             */
            ImageArraySlice imageSlice = {};
            /**
             * @brief Destination offset in the image.
             */
            Offset3D imageOffset = {};
            /**
             * @brief Size of the destination image region to fill.
             */
            Extent3D imageExtent = {};

            /**
             * @brief Aligned buffer row width. If 0, tight alignment is assumed (not supported on all APIs)
             */
            u32 rowPitch = {};

            PYRO_NODISCARD bool operator==(const CopyBufferToImageInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CopyBufferToImageInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for copying an image to a buffer.
         */
        struct CopyImageToBufferInfo {
            /**
             * @brief Source image handle. *MUST* be non-null.
             */
            Image image = PYRO_NULL_IMAGE;
            /**
             * @brief Source image slice to copy from.
             */
            ImageArraySlice imageSlice = {};
            /**
             * @brief Source offset in the image.
             */
            Offset3D imageOffset = {};
            /**
             * @brief Size of the source image region to copy.
             */
            Extent3D imageExtent = {};
            /**
             * @brief Destination buffer handle. *MUST* be non-null.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Destination buffer offset in bytes.
             */
            DeviceSize bufferOffset = {};

            /**
             * @brief Aligned buffer row width. If 0, tight alignment is assumed (not supported on all APIs)
             */
            u32 rowPitch = {};

            PYRO_NODISCARD bool operator==(const CopyImageToBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CopyImageToBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for copying one image to another image.
         */
        struct CopyImageToImageInfo {
            /**
             * @brief Source image handle. *MUST* be non-null.
             */
            Image srcImage = PYRO_NULL_IMAGE;
            /**
             * @brief Destination image handle. *MUST* be non-null.
             */
            Image dstImage = PYRO_NULL_IMAGE;
            /**
             * @brief Source image slice to copy from.
             */
            ImageArraySlice srcImageSlice = {};
            /**
             * @brief Source offset in the image.
             */
            Offset3D srcOffset = {};
            /**
             * @brief Destination image slice to copy into.
             */
            ImageArraySlice dstImageSlice = {};
            /**
             * @brief Destination offset in the image.
             */
            Offset3D dstOffset = {};
            /**
             * @brief Size of the image region to copy.
             */
            Extent3D extent = {};

            PYRO_NODISCARD bool operator==(const CopyImageToImageInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CopyImageToImageInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for blitting (scaling/filtering) one image to another.
         * @note Despite the srcImageBox and dstImageBox being a Box3D, this function currently REQUIRES the images to be 2D/2DArray
         */
        struct BlitImageToImageInfo {
            /**
             * @brief Source image handle. *MUST* be non-null.
             */
            Image srcImage = PYRO_NULL_IMAGE;
            /**
             * @brief Destination image handle. *MUST* be non-null.
             */
            Image dstImage = PYRO_NULL_IMAGE;
            /**
             * @brief Source image slice to blit from.
             */
            ImageArraySlice srcImageSlice = {};
            /**
             * @brief Source box region in the image.
             */
            Box3D srcImageBox = {};
            /**
             * @brief Destination image slice to blit into.
             */
            ImageArraySlice dstImageSlice = {};
            /**
             * @brief Destination box region in the image.
             */
            Box3D dstImageBox = {};
            /**
             * @brief Filtering mode applied during blit.
             */
            Filter filter = {};

            PYRO_NODISCARD bool operator==(const BlitImageToImageInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BlitImageToImageInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };


        /**
         * @brief Parameters for cleaing a UAV
         */
        struct ClearUnorderedAccessViewInfo {
            /**
             * @brief UAV handle. *MUST* be non-null
             */
            UnorderedAccessId view = PYRO_NULL_UAV;
            /**
             * @brief Clear value of the UAV. If an image, the 4 components will be used to clear.
             *
             * If a buffer, only the first component's value will be used to clear.
             */
            ColorClearValue clearValue = {};

            PYRO_NODISCARD bool operator==(const ClearUnorderedAccessViewInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ClearUnorderedAccessViewInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for updating a writable buffer
         */
        struct UpdateBufferInfo {
            /**
             * @brief Destination buffer handle. *MUST* be non-null.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Destination buffer region. By default, this is the entire region.
             *
             * This *MUST* be DWORD aligned and region must *NOT* exceed 65536 bytes!
             */
            BufferRegion region = {};
            /**
             * @brief The source data.
             * The memory size must minimally match the region defined by the region.
             */
            const void* data = {};

            PYRO_NODISCARD bool operator==(const UpdateBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const UpdateBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters describing a buffer memory barrier.
         *
         * Used to synchronize access to buffer resources between pipeline stages or queues.
         */
        struct BufferMemoryBarrierInfo {
            /// @brief Buffer handle to apply the barrier to.
            Buffer buffer = PYRO_NULL_BUFFER;

            /// @brief Region of the buffer affected by this barrier.
            BufferRegion region = {};

            /// @brief Source access flags (before the barrier).
            Access srcAccess = AccessConsts::NONE;

            /// @brief Destination access flags (after the barrier).
            Access dstAccess = AccessConsts::NONE;

            /// @brief Buffer layout before the barrier. If `Undefined`, contents may not be preserved
            BufferLayout srcLayout = BufferLayout::Undefined;

            /// @brief Buffer layout after the barrier. *MUST* not be `Undefined`
            BufferLayout dstLayout = BufferLayout::Undefined;

            PYRO_NODISCARD bool operator==(const BufferMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const BufferMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters describing an image memory barrier.
         *
         * Used to synchronize access to image resources between pipeline stages, layouts, or queues.
         */
        struct ImageMemoryBarrierInfo {
            /// @brief Image handle to apply the barrier to.
            Image image = PYRO_NULL_IMAGE;

            /// @brief Mip and array slice of the image affected by this barrier.
            ImageMipArraySlice imageSlice = {};

            /// @brief Source access flags (before the barrier).
            Access srcAccess = AccessConsts::NONE;

            /// @brief Destination access flags (after the barrier).
            Access dstAccess = AccessConsts::NONE;

            /// @brief Image layout before the barrier. If `Undefined`, contents may not be preserved
            ImageLayout srcLayout = ImageLayout::Undefined;

            /// @brief Image layout after the barrier. *MUST* not be `Undefined`
            ImageLayout dstLayout = ImageLayout::Undefined;

            PYRO_NODISCARD bool operator==(const ImageMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ImageMemoryBarrierInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for invalidating timestamp queries on the command buffer.
         */
        struct InvalidateTimestampQueryInfo {
            /**
             * @brief A NON-NULL Query pool to invalidate a range.
             */
            ITimestampQueryPool* queryPool = nullptr;
            /**
             * @brief First query to be invalidated.
             */
            u32 firstQuery = {};
            /**
             * @brief How many queries to invalidate.
             */
            u32 queryCount = {};
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for writing a timestamp on the command buffer.
         */
        struct WriteTimestampInfo {
            /**
             * @brief A NON-NULL Query pool to write to.
             */
            ITimestampQueryPool* queryPool = nullptr;
            /**
             * @brief During which pipeline stage the timestamp should be written.
             */
            PipelineStageFlags stage = {};
            /**
             * @brief Which query should be written to.
             */
            u32 queryIndex = {};
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for labeling a command region.
         */
        struct CommandLabelInfo {
            /**
             * @brief Color used to visually distinguish the label in debugging tools.
             */
            LabelColor labelColor = LabelColor::WHITE;
            /**
             * @brief Name for the label, useful for profiling or debugging.
             */
            eastl::string_view name = {};

            PYRO_NODISCARD bool operator==(const CommandLabelInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const CommandLabelInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Specifies the resolve mode for multisample attachments.
         */
        enum struct ResolveMode : u32 {
            None = 0,       ///< No resolve operation is performed.
            SampleZero = 1, ///< Takes the value from sample 0.
            Average = 2,    ///< Averages all samples.
            Min = 4,        ///< Takes the minimum value across samples.
            Max = 8,        ///< Takes the maximum value across samples.
        };

        /**
         * @brief Parameters for resolving a multisample attachment.
         */
        struct AttachmentResolveInfo {
            /**
             * @brief Resolve mode to use. Defaults to averaging all samples.
             */
            ResolveMode mode = ResolveMode::Average;
            /**
             * @brief Destination render target for the resolved data. *MUST* be non-null.
             */
            RenderTarget target = {};

            PYRO_NODISCARD bool operator==(const AttachmentResolveInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const AttachmentResolveInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for binding a color attachment in a render pass.
         */
        struct ColorAttachmentInfo {
            /**
             * @brief Target render surface. *MUST* be non-null.
             */
            RenderTarget target = {};
            /**
             * @brief Load operation for the attachment (e.g., clear, load, or don't care).
             */
            AttachmentLoadOp loadOp = AttachmentLoadOp::DontCare;
            /**
             * @brief Store operation for the attachment (e.g., store results or discard).
             */
            AttachmentStoreOp storeOp = AttachmentStoreOp::DontCare;
            /**
             * @brief Clear color value if loadOp is set to clear.
             */
            ColorClearValue clearValue = {};
            /**
             * @brief Optional resolve information for multisampled attachments.
             */
            eastl::optional<AttachmentResolveInfo> resolve = {};

            PYRO_NODISCARD bool operator==(const ColorAttachmentInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ColorAttachmentInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for binding a depth-stencil attachment in a render pass.
         */
        struct DepthStencilAttachmentInfo {
            /**
             * @brief Target render surface for depth-stencil output. *MUST* be non-null.
             */
            RenderTarget target = {};
            /**
             * @brief Load operation for the depth aspect.
             */
            AttachmentLoadOp depthLoadOp = AttachmentLoadOp::DontCare;
            /**
             * @brief Store operation for the depth aspect.
             */
            AttachmentStoreOp depthStoreOp = AttachmentStoreOp::DontCare;
            /**
             * @brief Load operation for the stencil aspect.
             */
            AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::DontCare;
            /**
             * @brief Store operation for the stencil aspect.
             */
            AttachmentStoreOp stencilStoreOp = AttachmentStoreOp::DontCare;
            /**
             * @brief Clear values for depth and stencil aspects if loadOps are set to clear.
             */
            DepthStencilClearValue clearValue = {};

            PYRO_NODISCARD bool operator==(const DepthStencilAttachmentInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DepthStencilAttachmentInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for beginning a render pass.
         */
        struct RenderPassBeginInfo {
            /**
             * @brief List of color attachments used in this render pass.
             */
            eastl::fixed_vector<ColorAttachmentInfo, Limits::MAX_BOUND_COLOR_TARGETS> colorAttachments = {};
            /**
             * @brief Optional depth-stencil attachment for this render pass.
             */
            eastl::optional<DepthStencilAttachmentInfo> depthStencilAttachment = {};
            /**
             * @brief Render area defining the output region in the framebuffer.
             */
            Rect2D renderArea = {};

            PYRO_NODISCARD bool operator==(const RenderPassBeginInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const RenderPassBeginInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for pushing constant data into a pipeline.
         */
        struct PushConstantInfo {
            /**
             * @brief Pointer to constant data to be uploaded.
             * @related size Must contain at least this amount of bytes of memory.
             */
            const void* data = {};
            /**
             * @brief Size of the constant data in bytes. This *MUST* be DWORD aligned
             */
            u32 size = {};
            /**
             * @brief Offset in the push constant range to write to. This *MUST* be DWORD aligned
             */
            u32 offset = {};

            PYRO_NODISCARD bool operator==(const PushConstantInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const PushConstantInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for setting a uniform buffer view in a pipeline.
         */
        struct SetUniformBufferViewInfo {
            /**
             * @brief Binding slot in the pipeline.
             */
            u32 slot = {};
            /**
             * @brief Buffer resource to bind. *MUST* be non-null if used.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Region of the buffer to bind.
             */
            BufferRegion region = {};
            /**
             * @brief Pipeline bind point (e.g., graphics or compute).
             */
            PipelineBindPoint bindPoint = {};

            PYRO_NODISCARD bool operator==(const SetUniformBufferViewInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SetUniformBufferViewInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for setting an unordered access view (UAV) in a pipeline.
         */
        struct SetUnorderedAccessViewInfo {
            /**
             * @brief Binding slot in the pipeline.
             */
            u32 slot = {};
            /**
             * @brief UAV resource to bind. *MUST* be non-null if used.
             */
            UnorderedAccessId view = PYRO_NULL_UAV;
            /**
             * @brief Pipeline bind point (e.g., graphics or compute).
             */
            PipelineBindPoint bindPoint = {};

            PYRO_NODISCARD bool operator==(const SetUnorderedAccessViewInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SetUnorderedAccessViewInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for binding a vertex buffer.
         */
        struct SetVertexBufferInfo {
            /**
             * @brief Binding slot index for the vertex buffer.
             */
            u32 slot = {};
            /**
             * @brief Vertex buffer handle. *MUST* be non-null if used.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Offset in the buffer where vertex data begins.
             */
            DeviceSize offset = {};

            PYRO_NODISCARD bool operator==(const SetVertexBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SetVertexBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        /**
         * @brief Parameters for binding an index buffer.
         */
        struct SetIndexBufferInfo {
            /**
             * @brief Index buffer handle. *MUST* be non-null if used.
             */
            Buffer buffer = PYRO_NULL_BUFFER;
            /**
             * @brief Offset in the buffer where index data begins.
             */
            DeviceSize offset = {};
            /**
             * @brief Type of indices stored in the buffer (e.g., 16-bit or 32-bit).
             */
            IndexType indexType = IndexType::Uint32;

            PYRO_NODISCARD bool operator==(const SetIndexBufferInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const SetIndexBufferInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DrawInfo {
            u32 vertexCount = {};
            u32 instanceCount = 1;
            u32 firstVertex = {};
            u32 firstInstance = {};

            PYRO_NODISCARD bool operator==(const DrawInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DrawInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DrawIndexedInfo {
            u32 indexCount = {};
            u32 instanceCount = 1;
            u32 firstIndex = {};
            i32 vertexOffset = {};
            u32 firstInstance = {};

            PYRO_NODISCARD bool operator==(const DrawIndexedInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DrawIndexedInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DrawIndirectInfo {
            Buffer indirectBuffer = PYRO_NULL_BUFFER;
            DeviceSize indirectBufferOffset = {};
            u32 drawCount = 1;
            u32 drawCommandStride = sizeof(DrawArgumentBuffer);

            PYRO_NODISCARD bool operator==(const DrawIndirectInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DrawIndirectInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DrawIndexedIndirectInfo {
            Buffer indirectBuffer = PYRO_NULL_BUFFER;
            DeviceSize indirectBufferOffset = {};
            u32 drawCount = 1;
            u32 drawCommandStride = sizeof(DrawIndexedArgumentBuffer);

            PYRO_NODISCARD bool operator==(const DrawIndexedIndirectInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DrawIndexedIndirectInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DispatchInfo {
            u32 x = 1;
            u32 y = 1;
            u32 z = 1;

            PYRO_NODISCARD bool operator==(const DispatchInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DispatchInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct DispatchIndirectInfo {
            Buffer indirectBuffer = PYRO_NULL_BUFFER;
            DeviceSize indirectBufferOffset = {};
            // u32 dispatchCount = 1;
            // u32 dispatchCommandStride = sizeof(DispatchArgumentBuffer);

            PYRO_NODISCARD bool operator==(const DispatchIndirectInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const DispatchIndirectInfo&) const = default;
            PYRO_NODISCARD eastl::string ToString(usize indentation = 0) const;
        };

        struct BuildAccelerationStructuresInfo {
            eastl::span<TlasBuildInfo const> tlasBuildInfos = {};
            eastl::span<BlasBuildInfo const> blasBuildInfos = {};
        };

        /**
         * @brief Interface for recording and submitting GPU commands.
         *
         * Implementations of this interface record commands into a command buffer,
         * which can later be submitted to a ICommandQueue for execution.
         */
        struct ICommandBuffer {
            ICommandBuffer() = default;

            // ---------------------------------------------------------------------
            // Copy / Transfer Commands
            // ---------------------------------------------------------------------

            /**
             * @brief Copies data from one buffer to another. *MUST* be called outside of a renderpass
             */
            virtual void CopyBufferToBuffer(const CopyBufferToBufferInfo& info) = 0;

            /**
             * @brief Copies data from a buffer to an image. *MUST* be called outside of a renderpass
             */
            virtual void CopyBufferToImage(const CopyBufferToImageInfo& info) = 0;

            /**
             * @brief Copies data from an image to a buffer. *MUST* be called outside of a renderpass
             */
            virtual void CopyImageToBuffer(const CopyImageToBufferInfo& info) = 0;

            /**
             * @brief Copies data between two images. *MUST* be called outside of a renderpass
             */
            virtual void CopyImageToImage(const CopyImageToImageInfo& info) = 0;

            /**
             * @brief Performs an image blit operation, optionally scaling and filtering. *MUST* be called outside of a renderpass
             */
            virtual void BlitImageToImage(const BlitImageToImageInfo& info) = 0;

            /**
             * @brief Clears an unordered access view (UAV) to a specified value. *MUST* be called outside of a renderpass
             */
            virtual void ClearUnorderedAccessView(const ClearUnorderedAccessViewInfo& info) = 0;

            /**
             * @brief Updates a buffer with user-provided data. *MUST* be called outside of a renderpass
             */
            virtual void UpdateBuffer(const UpdateBufferInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Memory Barriers
            // ---------------------------------------------------------------------

            /**
             * @brief Inserts a barrier for synchronizing buffer memory accesses. *MUST* be called outside of a renderpass
             */
            virtual void BufferBarrier(const BufferMemoryBarrierInfo& info) = 0;

            /**
             * @brief Inserts a barrier for synchronizing image memory accesses. *MUST* be called outside of a renderpass
             */
            virtual void ImageBarrier(const ImageMemoryBarrierInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Synchronization
            // ---------------------------------------------------------------------

            /**
             * @brief Transfers ownership of a buffer to another queue.
             *        The resource can no longer be used in the current queue after this call.
             *
             * @param buffer  The buffer whose ownership is being transferred.
             * @param dstQueue The destination queue that will take ownership of the buffer.
             */
            virtual void TransferBufferOwnership(Buffer buffer, ICommandQueue* dstQueue) = 0;

            /**
             * @brief Transfers ownership of an image to another queue.
             *        The resource can no longer be used in the current queue after this call.
             *
             * @param image   The image whose ownership is being transferred.
             * @param dstQueue The destination queue that will take ownership of the image.
             */
            virtual void TransferImageOwnership(Image image, ICommandQueue* dstQueue) = 0;

            /**
             * @brief Acquires ownership of a buffer from another queue.
             *        The resource can only be used in the current queue after this call.
             *
             * @param buffer   The buffer whose ownership is being acquired.
             * @param srcQueue The source queue that previously owned the buffer.
             */
            virtual void AcquireBufferOwnership(Buffer buffer, ICommandQueue* srcQueue) = 0;

            /**
             * @brief Acquires ownership of an image from another queue.
             *        The resource can only be used in the current queue after this call.
             *
             * @param image    The image whose ownership is being acquired.
             * @param srcQueue The source queue that previously owned the image.
             */
            virtual void AcquireImageOwnership(Image image, ICommandQueue* srcQueue) = 0;

            // ---------------------------------------------------------------------
            // Profiling & Debugging
            // ---------------------------------------------------------------------

            /**
             * @brief Invalidates the timestamps of the query pool. Required before using in a frame.
             */
            virtual void InvalidateTimestampQuery(const InvalidateTimestampQueryInfo& info) = 0;
            /**
             * @brief Writes a timestamp into a query pool for GPU profiling.
             */
            virtual void WriteTimestamp(const WriteTimestampInfo& info) = 0;

            /**
             * @brief Begins a labeled region for debugging or profiling. This can be nested.
             */
            virtual void BeginLabel(const CommandLabelInfo& info) = 0;

            /**
             * @brief Ends the current labeled region.
             */
            virtual void EndLabel() = 0;

            // ---------------------------------------------------------------------
            // Render Pass
            // ---------------------------------------------------------------------

            /**
             * @brief Begins a render pass with the specified attachments. EndRenderPass() *MUST* be called to complete.
             */
            virtual void BeginRenderPass(const RenderPassBeginInfo& info) = 0;

            /**
             * @brief Ends the current render pass.
             */
            virtual void EndRenderPass() = 0;

            // ---------------------------------------------------------------------
            // Constants and Pipeline State
            // ---------------------------------------------------------------------

            /**
             * @brief Pushes raw constant data.
             */
            virtual void PushConstantVPtr(const PushConstantInfo& info) = 0;

            /**
             * @brief Pushes typed constant data.
             */
            template <StandardLayoutConcept T>
            PYRO_FORCEINLINE void PushConstant(const T& constant, const u32 offset = 0) {
                PushConstantVPtr({
                    .data = static_cast<const void*>(&constant),
                    .size = static_cast<u32>(sizeof(T)),
                    .offset = offset,
                });
            }

            /**
             * @brief Binds a uniform buffer view.
             */
            virtual void SetUniformBufferView(const SetUniformBufferViewInfo& info) = 0;

            /**
             * @brief Binds an unordered access view (UAV).
             */
            virtual void SetUnorderedAccessView(const SetUnorderedAccessViewInfo& info) = 0;

            /**
             * @brief Sets the rasterization pipeline state.
             */
            virtual void SetRasterPipeline(RasterPipeline pipeline) = 0;

            /**
             * @brief Sets the compute pipeline state.
             */
            virtual void SetComputePipeline(ComputePipeline pipeline) = 0;

            // ---------------------------------------------------------------------
            // Viewport & Buffers
            // ---------------------------------------------------------------------

            /**
             * @brief Sets the active viewport for rendering. Not required to be called after BeginRenderPass()
             */
            virtual void SetViewport(const ViewportInfo& info) = 0;

            /**
             * @brief Sets the active scissor rectangle for rendering. Not required to be called after BeginRenderPass()
             */
            virtual void SetScissor(const Rect2D& info) = 0;

            /**
             * @brief Binds a vertex buffer.
             */
            virtual void SetVertexBuffer(const SetVertexBufferInfo& info) = 0;

            /**
             * @brief Binds an index buffer.
             */
            virtual void SetIndexBuffer(const SetIndexBufferInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Drawing
            // ---------------------------------------------------------------------

            /**
             * @brief Issues a non-indexed draw call. *MUST* be called inside of a renderpass
             */
            virtual void Draw(const DrawInfo& info) = 0;

            /**
             * @brief Issues an indexed draw call. *MUST* be called inside of a renderpass
             */
            virtual void DrawIndexed(const DrawIndexedInfo& info) = 0;

            /**
             * @brief Issues indirect draw commands from a buffer. *MUST* be called inside of a renderpass
             */
            virtual void DrawIndirect(const DrawIndirectInfo& info) = 0;

            /**
             * @brief Issues indirect indexed draw commands from a buffer. *MUST* be called inside of a renderpass
             */
            virtual void DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) = 0;

            // ---------------------------------------------------------------------
            // Compute
            // ---------------------------------------------------------------------

            /**
             * @brief Dispatches a compute shader workload. *MUST* be called outside of a renderpass
             */
            virtual void Dispatch(const DispatchInfo& info) = 0;

            /**
             * @brief Dispatches compute workloads using indirect parameters from a buffer. *MUST* be called outside of a renderpass
             */
            virtual void DispatchIndirect(const DispatchIndirectInfo& info) = 0;

            
            // ---------------------------------------------------------------------
            // Ray tracing and Acceleration structures
            // ---------------------------------------------------------------------

            virtual void BuildAccelerationStructures(const BuildAccelerationStructuresInfo& info) = 0;


            // ---------------------------------------------------------------------
            // Completion
            // ---------------------------------------------------------------------

            /**
             * @brief Marks the command buffer as complete and ready for submission.
             * After this, no more commands are accepted.
             */
            virtual void Complete() = 0;

        protected:
            virtual ~ICommandBuffer() = default;
            friend struct IDevice;
        };


    } // namespace RHI
} // namespace PyroshockStudios