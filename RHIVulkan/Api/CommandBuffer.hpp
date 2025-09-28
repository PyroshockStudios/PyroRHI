#pragma once
#include <EASTL/unique_ptr.h>

#include <RHIVulkan/Api/GPUResourcePool.hpp>
#include <RHIVulkan/Core.hpp>
#include <PyroRHI/Api/ICommandBuffer.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;
        struct CommandBufferZombieInfo {
            eastl::vector<ZombieDeleter> zombies = {};
        };

        struct CommandBufferPool {
            eastl::pair<VkCommandPool, VkCommandBuffer> Get(VulkanDevice* device);
            void PutBack(eastl::pair<VkCommandPool, VkCommandBuffer> poolAndBuffer);
            void Cleanup(VulkanDevice* device);

            eastl::vector<eastl::pair<VkCommandPool, VkCommandBuffer>> poolAndBuffers = {};
        };

        class VulkanCommandBuffer : public ICommandBuffer, DeleteCopy, DeleteMove {
        public:
            VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, VkCommandBuffer buffer, const CommandBufferInfo& info);
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

            void SignalEvent(const EventSignalInfo& info) override;
            void WaitEvents(const eastl::span<const EventWaitInfo>& infos) override;
            void WaitEvent(const EventWaitInfo& info) override;
            void ResetEvent(const ResetEventInfo& info) override;

            void DestroyDeviceMemoryDeferred(DeviceMemory memory) override;
            void DestroyBufferDeferred(Buffer buffer) override;
            void DestroyImageDeferred(Image image) override;
            void DestroyShaderResourceDeferred(ShaderResourceId srv) override;
            void DestroyUnorderedAccessDeferred(UnorderedAccessId uav) override;
            void DestroySamplerDeferred(SamplerId sampler) override;
            void DestroyRenderTargetDeferred(RenderTarget renderTarget) override;
            void DestroyRasterPipelineDeferred(RasterPipeline pipeline) override;
            void DestroyComputePipelineDeferred(ComputePipeline pipeline) override;

            void WriteTimestamp(const WriteTimestampInfo& info) override;
            void ResetTimestamps(const ResetTimestampsInfo& info) override;
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
            void Complete() override;

            VkCommandBuffer GetVkCommandBuffer() {
                return mCommandBuffer;
            }
            VkCommandPool GetVkCommandPool() {
                return mCommandPool;
            }

        private:
            inline void FlushBarriers();
            // TODO
            // void FlushPipelineLayout();

            eastl::unique_ptr<CommandBufferZombieInfo> mZombieInfo = eastl::make_unique<CommandBufferZombieInfo>();
            eastl::vector<VkBufferMemoryBarrier2> mBufferBarriers = {};
            eastl::vector<VkImageMemoryBarrier2> mImageBarriers = {};
            bool mCompleted = false;

            VulkanDevice* mDevice;
            VkCommandPool mCommandPool;
            VkCommandBuffer mCommandBuffer;
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios