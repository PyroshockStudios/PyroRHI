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
#include <D3D12MemAlloc.h>
#include <EASTL/atomic.h>
#include <EASTL/hash_map.h>
#include <EASTL/vector.h>
#include <PyroCommon/Logger.hpp>
#undef OPAQUE
#include <PyroRHI/Api/AccelerationStructure.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <RHIDX12/Core.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        struct DescriptorTableInfo {
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {};
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor = {};
            ComPtr<ID3D12DescriptorHeap> mHeap = {};

            PYRO_NODISCARD PYRO_FORCEINLINE bool operator==(const DescriptorTableInfo& other) const {
                return mHeap.Get() == other.mHeap.Get();
            }
            PYRO_NODISCARD PYRO_FORCEINLINE bool operator!=(const DescriptorTableInfo& other) const {
                return mHeap.Get() != other.mHeap.Get();
            }
        };

        class D3DDevice;

        struct ZombieDeleter {
            void* resource = {};
            FunctionPtr<void(D3DDevice* device, void* resource)> deleter = {};
        };

        template <typename TInfo>
        struct D3DHeapManager {
        public:
            D3DHeapManager(ID3D12Device* device, UINT maxDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool gpuVisible, const char* debugName) : mDebugName(debugName) {
                Logger::Debug(gDX12Sink, "Initialising Resource Pool named '{}'", debugName);
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = maxDescriptors;
                heapDesc.Type = heapType;
                heapDesc.Flags = gpuVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                CheckD3DResult(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap)));
                D3DSetDebugName(mHeap, debugName);
                mIncSz = device->GetDescriptorHandleIncrementSize(heapType);
                mDescriptorBase = mHeap->GetCPUDescriptorHandleForHeapStart();
            }
            ~D3DHeapManager() {
                if (mTombstones.size() != mHeapCounter) {
                    Logger::Warn(gDX12Sink, "Leaked {} resources in Resource Pool named '{}'", mHeapCounter - mTombstones.size(), mDebugName);
                }
            }
            eastl::pair<GpuResourceId, TInfo&> AcquireSlot() {
                UINT slot, version = 0;
                if (mTombstones.size() == 1) {
                    slot = mHeapCounter++;
                    mSlots.push_back({});
                } else {
                    auto kv = mTombstones.back();
                    slot = kv.first;
                    version = kv.second;
                    mTombstones.pop_back();
                }
                return { { .index = slot, .version = version }, mSlots[slot] };
            }
            void ReleaseSlot(GpuResourceId handle) {
                mTombstones.emplace_back(handle.index, ++handle.version);
            }
            void ReleaseSlot(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
                mTombstones.emplace_back(static_cast<UINT>((handle.ptr - mDescriptorBase.ptr) / mIncSz), 0);
            }

            D3D12_CPU_DESCRIPTOR_HANDLE Resolve(GpuResourceId slot) {
                D3D12_CPU_DESCRIPTOR_HANDLE handle;
                handle.ptr = mDescriptorBase.ptr + slot.index * mIncSz;
                return handle;
            }

            TInfo& GetInfo(GpuResourceId slot) {
                return mSlots[slot.index];
            }

            D3D12_CPU_DESCRIPTOR_HANDLE HostHandle() {
                return mDescriptorBase;
            }
            D3D12_GPU_DESCRIPTOR_HANDLE DeviceHandle() {
                return mHeap->GetGPUDescriptorHandleForHeapStart();
            }
            ID3D12DescriptorHeap* InternalHeap() {
                return mHeap.Get();
            }

        private:
            eastl::vector<eastl::pair<UINT, UINT>> mTombstones = { { 1, 0 } };
            eastl::vector<TInfo> mSlots = { TInfo{} };
            D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorBase = {};
            UINT mHeapCounter = 1;
            ComPtr<ID3D12DescriptorHeap> mHeap = {};
            UINT mIncSz{};
            eastl::string mDebugName = {};
        };
        struct D3DMemoryBlockResourceData {
            MemoryBlockInfo info = {};
            ComPtr<ID3D12Heap> heap = {};
            ComPtr<D3D12MA::VirtualBlock> block = {};
            u32 debugRefs = 0;
        };
        struct D3DBufferResourceData {
            ComPtr<ID3D12Resource> resource = {};
            ComPtr<D3D12MA::Allocation> allocation = {};
            D3D12MA::VirtualAllocation virtualAlloc = {};
            D3D12_RESOURCE_DESC desc = {};
            BufferInfo info = {};
            u8* mappedMemory = nullptr;
            D3D12_RANGE range = {};
        };
        struct D3DImageResourceData {
            ComPtr<ID3D12Resource> resource = {};
            ComPtr<D3D12MA::Allocation> allocation = {};
            D3D12MA::VirtualAllocation virtualAlloc = {};
            D3D12_RESOURCE_DESC desc = {};
            ImageInfo info = {};
            // Blit Dst
            eastl::vector<GpuResourceId> blitImageRTVs = {};
            // Blit Src
            eastl::vector<DescriptorTableInfo> blitImageSRVHeaps = {};
        };

        struct D3DBlasData {
            BlasInfo info = {};
            ComPtr<ID3D12Resource> resource = {};        // The buffer containing the AS
            ComPtr<D3D12MA::Allocation> allocation = {}; // The D3D12MA allocation
            D3D12_GPU_VIRTUAL_ADDRESS address = 0;       // The GPU address
        };

        struct D3DTlasData {
            TlasInfo info = {};
            ComPtr<ID3D12Resource> resource = {};        // The buffer containing the AS
            ComPtr<D3D12MA::Allocation> allocation = {}; // The D3D12MA allocation
        };

        struct D3DRenderTargetData {
            u64 x;
        };

        class GPUResourcePool : DeleteCopy, DeleteMove {
        public:
            GPUResourcePool(D3DDevice* device, UINT maxRtvs, UINT maxDsvs, UINT maxSRVs, UINT maxUAVs, UINT maxSamplers);
            ~GPUResourcePool();

            eastl::pair<MemoryBlock, D3DMemoryBlockResourceData&> AllocMemoryBlock();
            eastl::pair<Buffer, D3DBufferResourceData&> AllocBuffer();
            eastl::pair<Image, D3DImageResourceData&> AllocImage();
            eastl::pair<BlasId, D3DBlasData&> AllocBlas();

            void ReleaseMemoryBlock(MemoryBlock buffer);
            void ReleaseBuffer(Buffer buffer);
            void ReleaseImage(Image image);
            void ReleaseBlas(BlasId blas);

            D3DMemoryBlockResourceData& Get(MemoryBlock handle);
            D3DBufferResourceData& Get(Buffer handle);
            D3DImageResourceData& Get(Image handle);
            D3DBlasData& Get(BlasId handle);

            // includes TLAS
            D3DHeapManager<eastl::variant<GpuResourceInfo, D3DTlasData>> mSRVHeap;
            D3DHeapManager<GpuResourceInfo> mUAVHeap;
            D3DHeapManager<SamplerInfo> mSamplerHeap;
            D3DHeapManager<D3DRenderTargetData> mRTVHeap;
            D3DHeapManager<D3DRenderTargetData> mDSVHeap;

        private:
            eastl::hash_map<MemoryBlock, D3DMemoryBlockResourceData> mMemoryBlockResources = {};
            eastl::hash_map<Buffer, D3DBufferResourceData> mBufferResources = {};
            eastl::hash_map<Image, D3DImageResourceData> mImageResources = {};
            eastl::hash_map<BlasId, D3DBlasData> mBlasResources = {};

            eastl::atomic<u32> mMemoryBlockCounter = 1;
            eastl::atomic<u32> mBufferCounter = 1;
            eastl::atomic<u32> mImageCounter = 1;
            eastl::atomic<u32> mBlasCounter = 1;
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios