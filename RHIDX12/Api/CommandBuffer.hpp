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
#include <RHIDX12/Api/GPUResource.hpp>
#include <RHIDX12/Api/Device.hpp>
#include <RHIDX12/Core.hpp>
#include <PyroRHI/Api/ICommandBuffer.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        class D3DRasterPipeline;
        class D3DTimestampQueryPool;
        class D3DCommandBuffer : public ICommandBuffer, DeleteCopy, DeleteMove {
        public:
            D3DCommandBuffer(D3DDevice* device, ComPtr<ID3D12GraphicsCommandList>&& commandList, ComPtr<ID3D12CommandAllocator>&& allocator);
            ~D3DCommandBuffer();

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
            void BeginLabel(const CommandLabelInfo& info) override;
            void EndLabel() override;
            void BeginRenderPass(const RenderPassBeginInfo& info) override;
            void EndRenderPass() override;
            void PushConstantVPtr(const PushConstantInfo& info) override;
            void SetUniformBufferView(const SetUniformBufferViewInfo& info) override;
            void SetUnorderedAccessView(const SetUnorderedAccessViewInfo& info) override;
            void SetRasterPipeline(RasterPipeline pipeline) override;
            void SetComputePipeline(ComputePipeline pipeline) override;
            void SetViewport(const ViewportInfo& info) override;
            void SetScissor(const Rect2D& info) override;
            void SetVertexBuffer(const SetVertexBufferInfo& info) override;
            void SetIndexBuffer(const SetIndexBufferInfo& info) override;
            void Draw(const DrawInfo& info) override;
            void DrawIndexed(const DrawIndexedInfo& info) override;
            void DrawIndirect(const DrawIndirectInfo& info) override;
            void DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) override;
            void Dispatch(const DispatchInfo& info) override;
            void DispatchIndirect(const DispatchIndirectInfo& info) override;
            void Complete() override;

            void Reset() {
                CheckD3DResult(mCommandList->Reset(mAllocator.Get(), nullptr));
            }

        public:
            ID3D12GraphicsCommandList* GetCommands() {
                return mCommandList.Get();
            }
            bool bUsedBefore = false;

            eastl::vector<ZombieDeleter> mDeferredDeleteOps = {};
            eastl::vector<LinearUploadBuffer*> mPendingReturnLinearUploadBuffers = {};
        private:
            inline void FlushPendingVertexBufferBinds();
            inline void FlushPendingUnorderedAccessViewBinds();

            CommandBufferInfo mInfo = {};
            D3DRasterPipeline* mCurrentRasterPipeline = {};
            bool bIsComputePipeline = false;
            bool bBlitImageState = false;
            eastl::hash_set<u32> mInvalidatedVertexBufferBindings = {};
            ComPtr<ID3D12GraphicsCommandList> mCommandList = {};
            ComPtr<ID3D12CommandAllocator> mAllocator = {};

            struct ResolveRenderTargetInfo {
                ID3D12Resource* src;
                UINT srcSubresource;
                ID3D12Resource* dst;
                UINT dstSubresource;
                DXGI_FORMAT format;
            };
            eastl::fixed_vector<ResolveRenderTargetInfo, 8> mRenderPassResolves = {};
            eastl::vector<D3D12_VERTEX_BUFFER_VIEW> mPendingVertexBufferBinds = {};
            UAVDescriptorTableCache mPendingUAVBinds = {};
            DescriptorTableInfo mGraphicsLastBoundUAVDescriptorTable = {};
            DescriptorTableInfo mComputeLastBoundUAVDescriptorTable = {};
            LinearUploadBuffer* mCurrentLinearUploadBuffer = {};
            eastl::hash_map<D3DTimestampQueryPool*, eastl::pair<u32, u32>> mPendingQueryPoolMinMaxResolves = {};
            D3DDevice* mDevice = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios