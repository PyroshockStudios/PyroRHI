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
#include <RHIDX12/Core.hpp>

#include <D3D12MemAlloc.h>
#include <EASTL/unique_ptr.h>
#include <PyroCommon/Util/HashCombine.hpp>
#include <PyroRHI/Api/IDevice.hpp>
#include <RHIDX12/Api/GPUResource.hpp>
#include <RHIDX12/Helper/LinearUploadBuffer.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DCommandQueue;
        class D3DCommandBuffer;
        class D3DDevice;

        struct UAVDescriptorTableCache {
            eastl::fixed_vector<UnorderedAccessId, Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS> boundUavs{};
            bool EffectivelyEmpty() const {
                for (auto& u : boundUavs) {
                    if (u != PYRO_NULL_UAV) {
                        return false;
                    }
                }
                return true;
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const UAVDescriptorTableCache&) const = default;
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const UAVDescriptorTableCache&) const = default;
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios
namespace eastl {

    template <>
    struct hash<PyroshockStudios::RHIDX12::UAVDescriptorTableCache> {
        PyroshockStudios::Types::usize operator()(const PyroshockStudios::RHIDX12::UAVDescriptorTableCache& k) const {
            using namespace PyroshockStudios;
            using namespace PyroshockStudios::Types;
            using namespace PyroshockStudios::Util;
            using namespace PyroshockStudios::RHIDX12;
            usize seed = 0xFAFAFFEE;
            for (const UnorderedAccessId& id : k.boundUavs) {
                HashCombine(seed, id);
            }
            return seed;
        }
    };
} // namespace eastl

namespace PyroshockStudios {
    namespace RHIDX12 {
        using CommandSignatureCacheHandle = u64;
        struct CommandSignatureDesc {
            D3D12_INDIRECT_ARGUMENT_TYPE type;
            UINT stride;
            operator CommandSignatureCacheHandle() const {
                return eastl::bit_cast<CommandSignatureCacheHandle>(*this);
            }
        };
        static_assert(sizeof(CommandSignatureDesc) == sizeof(u64), "Cache Command Signature description is bad");

        using QueueFenceSnapshot = eastl::vector<u64>;

        class D3DDevice : public IDevice, DeleteCopy, DeleteMove {
        public:
            D3DDevice(ComPtr<ID3D12Device>&& device, ComPtr<IDXGIFactory4>&& factory, ComPtr<IDXGIAdapter1>&& adapter);
            ~D3DDevice();

            bool IsMemoryBlockValid(MemoryBlock handle) const override;
            bool IsBufferValid(Buffer handle) const override;
            bool IsImageValid(Image handle) const override;
            bool IsShaderResourceValid(ShaderResourceId id) const override;
            bool IsUnorderedAccessValid(UnorderedAccessId id) const override;
            bool IsSamplerValid(SamplerId id) const override;
            const MemoryBlockInfo& GetMemoryBlockInfo(MemoryBlock memory) const override;
            const BufferInfo& GetBufferInfo(Buffer buffer) const override;
            const ImageInfo& GetImageInfo(Image image) const override;
            const GpuResourceInfo& GetShaderResourceInfo(ShaderResourceId id) const override;
            const GpuResourceInfo& GetUnorderedAccessInfo(UnorderedAccessId id) const override;
            const SamplerInfo& GetSamplerInfo(SamplerId id) const override;
            const RenderTargetInfo& GetRenderTargetInfo(RenderTarget renderTarget) const override;
            const RasterPipelineInfo& GetRasterPipelineInfo(RasterPipeline pipeline) const override;
            const ComputePipelineInfo& GetComputePipelineInfo(ComputePipeline pipeline) const override;
            const SemaphoreInfo& GetSemaphoreInfo(Semaphore semaphore) const override;
            const BlasInfo& GetBlasInfo(BlasId blas) const override;
            const TlasInfo& GetTlasInfo(TlasId tlas) const override;

            DeviceAddress BufferDeviceAddress(Buffer buffer) const override;
            u8* BufferHostAddress(Buffer buffer) const override;
            BlasAddress BlasInstanceAddress(BlasId blas) const override;
            DeviceSize ImageSizeRequirements(Image image) const override;
            u32 ImageSubresourceRowPitch(Image image, u32 rowWidth, ImageSlice slice) const override;

            AccelerationStructureBuildSizesInfo BlasSizeRequirements(const BlasBuildInfo& info) const override;
            AccelerationStructureBuildSizesInfo TlasSizeRequirements(const TlasBuildInfo& info) const override;

            MemoryBlock CreateMemoryBlock(const MemoryBlockInfo& info) override;
            Buffer CreateBuffer(const BufferInfo& info) override;
            Image CreateImage(const ImageInfo& info) override;
            ShaderResourceId CreateShaderResource(const GpuResourceInfo& info) override;
            UnorderedAccessId CreateUnorderedAccess(const GpuResourceInfo& info) override;
            SamplerId CreateSampler(const SamplerInfo& info) override;
            RenderTarget CreateRenderTarget(const RenderTargetInfo& info) override;
            RasterPipeline CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) override;
            ComputePipeline CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) override;
            ISwapChain* CreateSwapChain(const SwapChainInfo& info) override;
            Semaphore CreateSemaphore(const SemaphoreInfo& info) override;
            IFence* CreateFence(const FenceInfo& info) override;
            ITimestampQueryPool* CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) override;
            BlasId CreateBlas(const BlasInfo& info) override;
            TlasId CreateTlas(const TlasInfo& info) override;

            void DestroyMemoryBlock(MemoryBlock& memory, bool bDefer) override;
            void DestroyBuffer(Buffer& buffer, bool bDefer) override;
            void DestroyImage(Image& image, bool bDefer) override;
            void DestroyShaderResource(ShaderResourceId& srv, bool bDefer) override;
            void DestroyUnorderedAccess(UnorderedAccessId& uav, bool bDefer) override;
            void DestroySampler(SamplerId& sampler, bool bDefer) override;
            void DestroyRenderTarget(RenderTarget& renderTarget, bool bDefer) override;
            void DestroyRasterPipeline(RasterPipeline& pipeline, bool bDefer) override;
            void DestroyComputePipeline(ComputePipeline& pipeline, bool bDefer) override;
            void DestroySwapChain(ISwapChain*& swapChain, bool bDefer) override;
            void DestroySemaphore(Semaphore& semaphore, bool bDefer) override;
            void DestroyFence(IFence*& fence, bool bDefer) override;
            void DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool, bool bDefer) override;
            void DestroyBlas(BlasId& blas, bool bDefer) override;
            void DestroyTlas(TlasId& tlas, bool bDefer) override;

            eastl::optional<Format> PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) const override;
            eastl::span<ICommandQueue*> GetCommandQueues() override;
            ICommandQueue* GetPresentQueue() override;
            void WaitIdle() override;
            void SubmitQueue(const CommandQueueSubmitInfo& info) override;
            void PresentQueue(const CommandQueuePresentInfo& info) override;
            const DeviceInfo& Info() const override;
            const DevicePropertiesInfo& Properties() const override;
            const DeviceFeaturesInfo& Features() const override;
            DeviceStatusInfo Status() const override;

            void CollectGarbage() override;

            void ImageAddIfNecessaryBlitSupport(D3DImageResourceData& data);
            const DescriptorTableInfo& GetUnorderedAccessViewDescriptorTable(const UAVDescriptorTableCache& desc);
            ID3D12PipelineState* GetBlitImagePipeline(DXGI_FORMAT format, bool bArray);

            LinearUploadBuffer* GetLinearBufferAllocation(UINT64 minSize = 65536ULL);

            ID3D12CommandSignature* GetDrawCommandSignature() {
                return mIndirectDrawSignature.Get();
            }
            ID3D12CommandSignature* GetDrawIndexedCommandSignature() {
                return mIndirectDrawIndexedSignature.Get();
            }
            ID3D12CommandSignature* GetDispatchCommandSignature() {
                return mIndirectDispatchSignature.Get();
            }


            IDXGIFactory4* InternalFactory() {
                return mFactory.Get();
            }
            ID3D12Device* InternalDevice() {
                return mDevice.Get();
            }

            GPUResourcePool& ResourcePool() {
                return *mResourcePool;
            }

        private:
            QueueFenceSnapshot SnapshotQueueFenceValues() const;

            void WriteAllSRVDescriptorHeapCopies(ID3D12Resource* pResource,
                const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, GpuResourceId handle);
            void DestroyAllUAVDescriptorHeapCopies(UnorderedAccessId handle);

        private:
            void CheckFeatureSupport();
            void CreateCommandQueues();
            void InitializeResourcePool();
            void CreateDescriptorHeaps();
            void CreateGlobalRootSignature();
            void CreateBlitImageRootSignature();
            void CreateDefaultSamplerHeaps();
            void CreateCommandSignatures();
            void CreateMemoryAllocator();

            void PopulateDeviceInfo();
            void PopulateDeviceProperties();
            void PopulateDeviceFeatures();

            void DestroyCommandQueues();
            void DestroyUploadBuffers();
            void ReportDeviceRemovalReason();

        private:
            ComPtr<IDXGIAdapter1> mAdapter = {};
            ComPtr<IDXGIFactory4> mFactory = {};
            ComPtr<ID3D12Device> mDevice = {};
            ComPtr<D3D12MA::Allocator> mAllocator = {};

        public:
            ComPtr<ID3D12RootSignature> mRootSignature = {};

            ComPtr<ID3D12RootSignature> mBlitImageRootSignature = {};
            eastl::hash_map<UINT64, ComPtr<ID3D12PipelineState>> mBlitImagePipelineStates = {};
            DescriptorTableInfo mDefaultUAVDescriptorTable = {};
            DescriptorTableInfo mNearestSamplerDescriptorTable = {};
            DescriptorTableInfo mLinearSamplerDescriptorTable = {};

        private:
            eastl::vector<ICommandQueue*> mCommandQueueList = {};
            D3DCommandQueue* mCommandQueue = {};
            eastl::unique_ptr<GPUResourcePool> mResourcePool = {};

            eastl::vector<eastl::pair<UINT64 /*frames unused*/, LinearUploadBuffer*>> mAvailableLinearUploadBuffers = {};

            eastl::vector<eastl::pair<UINT64 /* cpu fence value @*/, D3DCommandQueue*>> mQueuePendingSubmits;

            eastl::vector<eastl::pair<QueueFenceSnapshot, ZombieDeleter>>
                mDeferredDeletes;

            CD3DX12FeatureSupport mDx12FeatureSupport{};

            DeviceInfo mInfo{};
            DevicePropertiesInfo mProperties{};
            DeviceFeaturesInfo mFeatures{};

            ComPtr<ID3D12CommandSignature> mIndirectDrawSignature = {};
            ComPtr<ID3D12CommandSignature> mIndirectDrawIndexedSignature = {};
            ComPtr<ID3D12CommandSignature> mIndirectDispatchSignature = {};
            eastl::hash_map<UAVDescriptorTableCache, eastl::pair<UINT64 /*frames unused*/, DescriptorTableInfo>> mUAVDescriptorTableCache = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios
