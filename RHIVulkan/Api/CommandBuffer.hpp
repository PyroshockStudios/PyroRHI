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
#include <EASTL/hash_set.h>
#include <EASTL/unique_ptr.h>

#include <PyroRHI/Api/ICommandBuffer.hpp>
#include <RHIVulkan/Api/GPUResourcePool.hpp>
#include <RHIVulkan/Core.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;
        class VulkanCommandQueue;
        class VulkanSwapChain;
        struct CommandBufferPool : DeleteCopy, DeleteMove {
            eastl::pair<VkCommandPool, VkCommandBuffer> Get(VulkanDevice* device, VulkanCommandQueue* queue);
            void PutBack(eastl::pair<VkCommandPool, VkCommandBuffer> poolAndBuffer);
            void Cleanup(VulkanDevice* device, VulkanCommandQueue* queue);

            eastl::vector<eastl::pair<VkCommandPool, VkCommandBuffer>> poolAndBuffers = {};
        };

        class VulkanCommandBuffer : public ICommandBuffer, DeleteCopy, DeleteMove {
        public:
            VulkanCommandBuffer(VulkanDevice* device, VulkanCommandQueue* queue, VkCommandPool pool, VkCommandBuffer buffer, const CommandBufferInfo& info);
            ~VulkanCommandBuffer();

            void CopyBufferToBuffer(const CopyBufferToBufferInfo& info) override;
            void CopyBufferToImage(const CopyBufferToImageInfo& info) override;
            void CopyImageToBuffer(const CopyImageToBufferInfo& info) override;
            void CopyImageToImage(const CopyImageToImageInfo& info) override;
            void BlitImageToImage(const BlitImageToImageInfo& info) override;
            void ClearUnorderedAccessView(const ClearUnorderedAccessViewInfo& info) override;
            void UpdateBuffer(const UpdateBufferInfo& info) override;
            void BufferBarrier(const BufferMemoryBarrierInfo& info) override;
            void ImageBarrier(const ImageMemoryBarrierInfo& info) override;
            void AccelerationStructureBarrier(const AccelerationStructureBarrierInfo& info) override;

            void TransferBufferOwnership(Buffer buffer, ICommandQueue* dstQueue) override;
            void TransferImageOwnership(Image image, ICommandQueue* dstQueue) override;
            void AcquireBufferOwnership(Buffer buffer, ICommandQueue* srcQueue) override;
            void AcquireImageOwnership(Image image, ICommandQueue* srcQueue) override;

            void InvalidateTimestampQuery(const InvalidateTimestampQueryInfo& info) override;
            void WriteTimestamp(const WriteTimestampInfo& info) override;
            void BeginLabel(const CommandLabelInfo& info) override;
            void EndLabel() override;
            void BeginRenderPass(const RenderPassBeginInfo& info) override;
            void EndRenderPass() override;
            void PushConstantVPtr(const PushConstantInfo& info) override;
            void SetUnorderedAccessView(const SetUnorderedAccessViewInfo& info) override;
            void SetUniformBufferView(const SetUniformBufferViewInfo& info) override;
            void SetRasterPipeline(RasterPipeline pipeline) override;
            void SetComputePipeline(ComputePipeline pipeline) override;
            void SetViewport(const ViewportInfo& info) override;
            void SetScissor(const Rect2D& info) override;
            void SetIndexBuffer(const SetIndexBufferInfo& info) override;
            void SetVertexBuffer(const SetVertexBufferInfo& info) override;
            void Draw(const DrawInfo& info) override;
            void DrawIndexed(const DrawIndexedInfo& info) override;
            void DrawIndirect(const DrawIndirectInfo& info) override;
            void DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) override;
            void Dispatch(const DispatchInfo& info) override;
            void DispatchIndirect(const DispatchIndirectInfo& info) override;
            void BuildAccelerationStructures(const BuildAccelerationStructuresInfo& info) override;
            void Complete() override;

            VkCommandBuffer GetVkCommandBuffer() {
                return mCommandBuffer;
            }
            VkCommandPool GetVkCommandPool() {
                return mCommandPool;
            }

            const eastl::hash_set<VulkanSwapChain*>& GetSwapchainReferences()const {
                return mSwapchainRefs;
            }
        private:
            inline void FlushBarriers();
            // TODO
            // void FlushPipelineLayout();
            inline void CheckIfSwapchainReference(const ImplImageSlot& slot);

            eastl::hash_set<VulkanSwapChain*> mSwapchainRefs;

            eastl::vector<VkMemoryBarrier2> mMemoryBarriers = {};
            eastl::vector<VkBufferMemoryBarrier2> mBufferBarriers = {};
            eastl::vector<VkImageMemoryBarrier2> mImageBarriers = {};
            bool mCompleted = false;

            VulkanDevice* mDevice;
            VulkanCommandQueue* mQueue;
            VkCommandPool mCommandPool;
            VkCommandBuffer mCommandBuffer;
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios