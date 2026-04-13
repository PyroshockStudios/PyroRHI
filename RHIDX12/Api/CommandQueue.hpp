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
#include <EASTL/atomic.h>
#include <PyroRHI/Api/ICommandQueue.hpp>
#include <PyroRHI/Common/AtomicQueue.hpp>
#include <PyroRHI/Common/AtomicVector.hpp>
#include <RHIDX12/Core.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DCommandBuffer;
        class D3DDevice;
        class D3DCommandQueue : public ICommandQueue, DeleteCopy, DeleteMove {
        public:
            D3DCommandQueue(D3DDevice* device, CommandQueueInfo&& info, ComPtr<ID3D12CommandQueue>&& queue);
            ~D3DCommandQueue();

            ICommandBuffer* GetCommandBuffer(const CommandBufferInfo& info) override;
            void WaitIdle() override;
            const CommandQueueInfo& Info() const override;
            f64 GetTimestampTickPeriodNs() const override;

            ID3D12CommandQueue* InternalQueue() {
                return mCommandQueue.Get();
            }
            void RestoreCommandBuffer(D3DCommandBuffer* cmb) {
                mPooledCommandBuffers.EmplaceBack(cmb, static_cast<UINT64>(mCurrentQueueFenceValue));
            }
          
            // Used for tracking which command buffers can be resurrected and when. Also for resource destruction
            void SignalQueueFence(UINT64 value);

            UINT64 GetFenceValue();

            UINT64 GetCpuValue() {
                return mCurrentQueueFenceValue;
            }
            UINT64 IncGetCpuValue() {
                return mCurrentQueueFenceValue.add_fetch(1);
            }

        private:
            D3DDevice* mDevice = nullptr;
            CommandQueueInfo mInfo = {};
            ComPtr<ID3D12CommandQueue> mCommandQueue = {};

            eastl::atomic<UINT64> mCurrentQueueFenceValue = 0;
            ComPtr<ID3D12Fence> mQueueTracker = {};

            Common::AtomicVector<eastl::pair<D3DCommandBuffer*, UINT64>> mPooledCommandBuffers = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios