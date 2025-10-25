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
#include <EASTL/unique_ptr.h>
#include <RHIDX12/Api/GPUResource.hpp>
#include <RHIDX12/Core.hpp>
#include <RHIDX12/Helper/LinearUploadBuffer.hpp>
#include <PyroCommon/Util/HashCombine.hpp>
#include <PyroRHI/Api/IDevice.hpp>
#include <D3D12MemAlloc.h>


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
            const GPUResourceInfo& GetShaderResourceInfo(ShaderResourceId id) const override;
            const GPUResourceInfo& GetUnorderedAccessInfo(UnorderedAccessId id) const override;
            const SamplerInfo& GetSamplerInfo(SamplerId id) const override;
            const RenderTargetInfo& GetRenderTargetInfo(RenderTarget renderTarget) const override;
            const RasterPipelineInfo& GetRasterPipelineInfo(RasterPipeline pipeline) const override;
            const ComputePipelineInfo& GetComputePipelineInfo(ComputePipeline pipeline) const override;
            const SemaphoreInfo& GetSemaphoreInfo(Semaphore semaphore) const override;
            const BLASInfo& GetBLASInfo(BLASId blas) const override;
            const TLASInfo& GetTLASInfo(TLASId tlas) const override;

            DeviceAddress BufferDeviceAddress(Buffer buffer) const override;
            u8* BufferHostAddress(Buffer buffer) const override;

            DeviceSize ImageSizeRequirements(Image image) const override;
            u32 ImageSubresourceRowPitch(Image image, ImageSlice slice, u32 rowWidth) const override;

            AccelerationStructureBuildSizesInfo BLASSizeRequirements(BLASId blas, eastl::span<const BLASGeometryInfo> blasGeometryInfo, u32 primitiveCount) const override;
            AccelerationStructureBuildSizesInfo TLASSizeRequirements(TLASId tlas, eastl::span<const TLASInstanceInfo> tlasInstanceInfo, u32 instanceCount) const override;

            MemoryBlock CreateMemoryBlock(const MemoryBlockInfo& info) override;
            Buffer CreateBuffer(const BufferInfo& info) override;
            Image CreateImage(const ImageInfo& info) override;
            ShaderResourceId CreateShaderResource(const GPUResourceInfo& info) override;
            UnorderedAccessId CreateUnorderedAccess(const GPUResourceInfo& info) override;
            SamplerId CreateSampler(const SamplerInfo& info) override;
            BLASId CreateBLAS(const BLASInfo& info) override;
            TLASId CreateTLAS(const TLASInfo& info) override;

            RenderTarget CreateRenderTarget(const RenderTargetInfo& info) override;
            RasterPipeline CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) override;
            ComputePipeline CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) override;
            ISwapChain* CreateSwapChain(const SwapChainInfo& info) override;
            Semaphore CreateSemaphore(const SemaphoreInfo& info) override;
            IFence* CreateFence(const FenceInfo& info) override;
            ITimestampQueryPool* CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) override;

            void DestroyMemoryBlock(MemoryBlock& memory) override;
            void DestroyBuffer(Buffer& buffer) override;
            void DestroyImage(Image& image) override;
            void DestroyShaderResource(ShaderResourceId& srv) override;
            void DestroyUnorderedAccess(UnorderedAccessId& uav) override;
            void DestroySampler(SamplerId& sampler) override;
            void DestroyBLAS(BLASId& blas) override;
            void DestroyTLAS(TLASId& tlas) override;

            void DestroyRenderTarget(RenderTarget& renderTarget) override;
            void DestroyRasterPipeline(RasterPipeline& pipeline) override;
            void DestroyComputePipeline(ComputePipeline& pipeline) override;
            void DestroySwapChain(ISwapChain*& swapChain) override;
            void DestroySemaphore(Semaphore& semaphore) override;
            void DestroyFence(IFence*& fence) override;
            void DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool) override;

            virtual eastl::optional<Format> PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) override;
            virtual eastl::span<ICommandQueue*> GetCommandQueues() override;
            virtual ICommandQueue* GetPresentQueue() override;
            void WaitIdle() override;
            void SubmitQueue(const CommandQueueSubmitInfo& info) override;
            void PresentQueue(const CommandQueuePresentInfo& info) override;
            const DeviceInfo& GetInfo() override;
            const DevicePropertiesInfo& GetProperties() override;

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
            void WriteAllSRVDescriptorHeapCopies(ID3D12Resource* pResource,
                const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, GPUResourceId handle);
            void DestroyAllUAVDescriptorHeapCopies(UnorderedAccessId handle);
            void CollectGarbage();

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
            eastl::vector<eastl::pair<UINT64 /*fence value*/, LinearUploadBuffer*>> mOccupiedLinearUploadBuffers = {};

            ComPtr<ID3D12Fence> mDeferredDeleterFence = {};
            UINT64 mNextDeferredDeleterValue = 1;
            eastl::vector<eastl::pair<UINT64 /*fence value*/, ZombieDeleter>> mDeferredDeletes;

            DevicePropertiesInfo mProperties{};

            ComPtr<ID3D12CommandSignature> mIndirectDrawSignature = {};
            ComPtr<ID3D12CommandSignature> mIndirectDrawIndexedSignature = {};
            ComPtr<ID3D12CommandSignature> mIndirectDispatchSignature = {};
            eastl::hash_map<UAVDescriptorTableCache, eastl::pair<UINT64 /*frames unused*/, DescriptorTableInfo>> mUAVDescriptorTableCache = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios
