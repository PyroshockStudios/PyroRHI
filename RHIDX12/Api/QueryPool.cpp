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

#include "QueryPool.hpp"
#include "Device.hpp"
#include <RHIDX12/D3DContext.hpp>

#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DTimestampQueryPool::D3DTimestampQueryPool(D3DDevice* device, const TimestampQueryPoolInfo& info)
            : mInfo(info), mDevice(device) {
            {
                D3D12_QUERY_HEAP_DESC desc{};
                desc.Count = info.queryCount;
                desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

                CheckD3DResult(device->InternalDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&mQueryPool)));
                D3DSetDebugName(mQueryPool, info.name.c_str());
            }



            // Map GPU buffer to CPU-visible memory and copy into mResultBuffer
            {
                D3D12_RESOURCE_DESC desc = {};
                desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                desc.Width = info.queryCount * sizeof(u64);
                desc.Height = 1;
                desc.DepthOrArraySize = 1;
                desc.MipLevels = 1;
                desc.Format = DXGI_FORMAT_UNKNOWN;
                desc.SampleDesc.Count = 1;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
               CheckD3DResult(device->InternalDevice()->CreateCommittedResource(
                    &heap,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_COPY_DEST, 
                    nullptr,
                    IID_PPV_ARGS(&mReadbackBuffer)));
                D3D12_RANGE range{};
                range.Begin = 0;
                range.End = desc.Width;

                CheckD3DResult(mReadbackBuffer->Map(0, &range, &mMappedResult));
                D3DSetDebugName(mQueryPool, (info.name + " (Readback Buffer)").c_str());
            }
        }
        D3DTimestampQueryPool::~D3DTimestampQueryPool() {
            mReadbackBuffer->Unmap(0, nullptr);
        }

        eastl::span<const u64> D3DTimestampQueryPool::GetTimestamps(u32 startIndex, u32 count) const {
            ASSERT((startIndex + count) <= mInfo.queryCount, "Timestamp query out of bounds!");
            return eastl::span<const u64>(static_cast<u64*>(mMappedResult) + startIndex, static_cast<u64*>(mMappedResult) + startIndex + count);
        }

    } // namespace RHIDX12
} // namespace PyroshockStudios