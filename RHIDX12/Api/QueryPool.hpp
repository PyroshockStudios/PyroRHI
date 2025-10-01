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
#include <PyroRHI/Api/IQueryPool.hpp>
#include <RHIDX12/Core.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;

        class D3DTimestampQueryPool : public ITimestampQueryPool, DeleteCopy, DeleteMove {
        public:
            D3DTimestampQueryPool(D3DDevice* device, const TimestampQueryPoolInfo& info);
            ~D3DTimestampQueryPool();
            const TimestampQueryPoolInfo& Info() const override {
                return mInfo;
            }

            eastl::span<const u64> GetTimestamps(u32 startIndex, u32 count) const override;

            ID3D12QueryHeap* GetInternalHeap() {
                return mQueryPool.Get();
            }
            ID3D12Resource* GetReadbackBuffer() {
                return mReadbackBuffer.Get();
            }

        private:
            mutable void* mMappedResult = nullptr;
            ComPtr<ID3D12QueryHeap> mQueryPool = {};
            ComPtr<ID3D12Resource> mReadbackBuffer = {};
            D3DDevice* mDevice;
            TimestampQueryPoolInfo mInfo;
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios