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
#include <PyroRHI/Api/IFence.hpp>
#include <PyroRHI/Api/Semaphore.hpp>
#include <PyroRHI/Api/Sync.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;

        class D3DSemaphore :  DeleteCopy, DeleteMove {
        public:
            D3DSemaphore(D3DDevice* device, SemaphoreInfo&& info);
            ~D3DSemaphore();

            const SemaphoreInfo& Info() const ;

            ID3D12Fence* InternalFence() {
                return mFence.Get();
            }
            PYRO_FORCEINLINE void Signal(ID3D12CommandQueue* queue) {
                ++mCurrentValue; // increment for each signal
                queue->Signal(mFence.Get(), mCurrentValue);
            }
            // GPU wait
            PYRO_FORCEINLINE void Wait(ID3D12CommandQueue* queue) {
                queue->Wait(mFence.Get(), mCurrentValue);
            }

        private:
            UINT64 mCurrentValue = 0;
            SemaphoreInfo mInfo = {};
            ComPtr<ID3D12Fence> mFence = {};
            D3DDevice* mDevice = {};
        };

        class D3DFence : public IFence, DeleteCopy, DeleteMove {
        public:
            D3DFence(D3DDevice* device, FenceInfo&& info);
            ~D3DFence();

            const FenceInfo& Info() const override;


            u64 Value() const override;
            void SetValue(u64 value) override;
            bool WaitForValue(u64 value, u64 timeoutNs) override;

            ID3D12Fence* InternalFence() {
                return mFence.Get();
            }

        private:
            UINT64 mCurrentValue = 0;
            HANDLE mEvent = {};
            FenceInfo mInfo = {};
            ComPtr<ID3D12Fence> mFence = {};
            D3DDevice* mDevice = {};
        };

    } // namespace RHIDX12
} // namespace PyroshockStudios