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
                IID_PPV_ARGS(&mUploadBuffer)), "Failed to create Linear Upload Buffer!");

            // Persistent mapping for CPU access
            CheckD3DResult(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedPtr)), "Failed to map Linear Upload Buffer!");
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