#include "LinearUploadBuffer.hpp"
#include <libassert/assert.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        LinearUploadBuffer::LinearUploadBuffer(ID3D12Device* device, UINT64 bufferSize) {
            mSize = PYRO_ALIGN(bufferSize, alignment);

            // Create a single big UPLOAD heap buffer
            CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
            CD3DX12_RESOURCE_DESC bufDesc = CD3DX12_RESOURCE_DESC::Buffer(
                mSize,
                D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
                0ULL);

            CheckD3DResult(device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &bufDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mUploadBuffer)));

            // Persistent mapping for CPU access
            CheckD3DResult(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedPtr)));
        }


        LinearUploadBuffer::~LinearUploadBuffer() {
            mUploadBuffer->Unmap(0, nullptr);
        }

        void* LinearUploadBuffer::Allocate(UINT64 size, UINT64& outOffset) {
            size = PYRO_ALIGN(size, alignment);

            if (mOffset + size > mSize) {
                // Wrap around
                mOffset = 0;
            }

            outOffset = mOffset;
            mOffset += size;
            return mMappedPtr + outOffset;
        }

    } // namespace RHIDX12
} // namespace PyroshockStudios