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
        GPUResourcePool::GPUResourcePool(D3DDevice* device, UINT maxRtvs, UINT maxDsvs, UINT maxSRVs, UINT maxUAVs, UINT maxSamplers)
            : mSRVHeap(device->InternalDevice(), maxSRVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, "Global SRV Heap"),
              mUAVHeap(device->InternalDevice(), maxUAVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, "Global UAV Heap"),
              mSamplerHeap(device->InternalDevice(), maxSamplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true, "Global Sampler Heap"),
              mRTVHeap(device->InternalDevice(), maxRtvs, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, "Global RTV Heap"),
              mDSVHeap(device->InternalDevice(), maxDsvs, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false, "Global DSV Heap")
        {
        }
        GPUResourcePool::~GPUResourcePool() {
        }

        struct ResourceHandle {
            u32 unused;
            u32 counter;
        };
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
        void GPUResourcePool::ReleaseBuffer(Buffer buffer) {
            ASSERT(mBufferResources.contains(buffer), "Double free occurred!");

            mBufferResources.erase(buffer);
        }
        void GPUResourcePool::ReleaseImage(Image image) {
            ASSERT(mImageResources.contains(image), "Double free occurred!");

            mImageResources.erase(image);
        }
        D3DBufferResourceData& GPUResourcePool::Get(Buffer handle) {
            ASSERT(mBufferResources.contains(handle), "Invalid handle!");
            return mBufferResources.at(handle);
        }
        D3DImageResourceData& GPUResourcePool::Get(Image handle) {
            ASSERT(mImageResources.contains(handle), "Invalid handle!");
            return mImageResources.at(handle);
        }
    }
}