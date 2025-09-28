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