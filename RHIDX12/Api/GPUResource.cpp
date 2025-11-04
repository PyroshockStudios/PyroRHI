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

#include "GPUResource.hpp"
#include "Device.hpp"
#include <libassert/assert.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        GPUResourcePool::GPUResourcePool(D3DDevice* device, UINT maxRtvs, UINT maxDsvs,
            UINT maxSRVs, UINT maxUAVs, UINT maxSamplers)
            : mSRVHeap(device->InternalDevice(), maxSRVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, "Global SRV Heap"),
              mUAVHeap(device->InternalDevice(), maxUAVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, "Global UAV Heap"),
              mSamplerHeap(device->InternalDevice(), maxSamplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true, "Global Sampler Heap"),
              mRTVHeap(device->InternalDevice(), maxRtvs, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, "Global RTV Heap"),
              mDSVHeap(device->InternalDevice(), maxDsvs, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false, "Global DSV Heap") {
        }
        GPUResourcePool::~GPUResourcePool() {
            if (mMemoryBlockResources.size() > 0) {
                Logger::Warn(gDX12Sink, "Leaked {} memory block resources!", mMemoryBlockResources.size());
                for (auto& [k, v] : mMemoryBlockResources) {
                    Logger::Warn(gDX12Sink, "    - memory block {}", v.info.name);
                }
            }
            if (mBufferResources.size() > 0) {
                Logger::Warn(gDX12Sink, "Leaked {} buffer resources!", mBufferResources.size());
                for (auto& [k, v] : mBufferResources) {
                    Logger::Warn(gDX12Sink, "    - buffer {}", v.info.name);
                }
            }
            if (mImageResources.size() > 0) {
                Logger::Warn(gDX12Sink, "Leaked {} image resources!", mImageResources.size());
                for (auto& [k, v] : mImageResources) {
                    Logger::Warn(gDX12Sink, "    - image {}", v.info.name);
                }
            }
        }

        struct ResourceHandle {
            u32 unused;
            u32 counter;
        };
        eastl::pair<MemoryBlock, D3DMemoryBlockResourceData&> GPUResourcePool::AllocMemoryBlock() {
            ResourceHandle handle;
            handle.unused = 0xDEADBEEF;
            handle.counter = mMemoryBlockCounter++;
            MemoryBlock buff = eastl::bit_cast<MemoryBlock>(handle);
            return { buff, mMemoryBlockResources[buff] };
        }
        eastl::pair<Buffer, D3DBufferResourceData&> GPUResourcePool::AllocBuffer() {
            ResourceHandle handle;
            handle.unused = 0xDEADBEEF;
            handle.counter = mBufferCounter++;
            Buffer buff = eastl::bit_cast<Buffer>(handle);
            return { buff, mBufferResources[buff] };
        }
        eastl::pair<Image, D3DImageResourceData&> GPUResourcePool::AllocImage() {
            ResourceHandle handle;
            handle.unused = 0xDEADBEEF;
            handle.counter = mImageCounter++;
            Image buff = eastl::bit_cast<Image>(handle);
            return { buff, mImageResources[buff] };
        }
        eastl::pair<BlasId, D3DBlasData&> GPUResourcePool::AllocBlas() {
            ResourceHandle handle;
            handle.unused = 0xDEADBEEF;
            handle.counter = mBlasCounter++;
            BlasId blas = eastl::bit_cast<BlasId>(handle);
            return { blas, mBlasResources[blas] };
        }
        void GPUResourcePool::ReleaseMemoryBlock(MemoryBlock memory) {
            ASSERT(mMemoryBlockResources.contains(memory), "Double free occurred!");

            mMemoryBlockResources.erase(memory);
        }
        void GPUResourcePool::ReleaseBuffer(Buffer buffer) {
            ASSERT(mBufferResources.contains(buffer), "Double free occurred!");

            mBufferResources.erase(buffer);
        }
        void GPUResourcePool::ReleaseImage(Image image) {
            ASSERT(mImageResources.contains(image), "Double free occurred!");

            mImageResources.erase(image);
        }
        void GPUResourcePool::ReleaseBlas(BlasId blas) {
            ASSERT(mBlasResources.contains(blas), "Double free occurred!");

            mBlasResources.erase(blas);
        }
        D3DMemoryBlockResourceData& GPUResourcePool::Get(MemoryBlock handle) {
            ASSERT(mMemoryBlockResources.contains(handle), "Invalid handle!");
            return mMemoryBlockResources.at(handle);
        }
        D3DBufferResourceData& GPUResourcePool::Get(Buffer handle) {
            ASSERT(mBufferResources.contains(handle), "Invalid handle!");
            return mBufferResources.at(handle);
        }
        D3DImageResourceData& GPUResourcePool::Get(Image handle) {
            ASSERT(mImageResources.contains(handle), "Invalid handle!");
            return mImageResources.at(handle);
        }
        D3DBlasData& GPUResourcePool::Get(BlasId handle) {
            ASSERT(mBlasResources.contains(handle), "Invalid handle!");
            return mBlasResources.at(handle);
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios