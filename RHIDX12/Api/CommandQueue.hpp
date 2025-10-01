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
#include <PyroRHI/Api/ICommandQueue.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DCommandBuffer;
        class D3DCommandQueue : public ICommandQueue, DeleteCopy, DeleteMove {
        public:
            D3DCommandQueue(CommandQueueInfo&& info, ComPtr<ID3D12CommandQueue>&& queue);
            ~D3DCommandQueue();

            void SubmitCommandBuffer(ICommandBuffer*& commandBuffer) override;
            void SubmitSwapChain(ISwapChain* swapChain) override;
            void WaitIdle() override;
            const CommandQueueInfo& Info() const override;
            f64 GetTimestampTickPeriodNs() const override;

            ID3D12CommandQueue* InternalQueue() {
                return mCommandQueue.Get();
            }

            eastl::vector<D3DCommandBuffer*> mSubmittedCommands = {};
            eastl::vector<ID3D12CommandList*> mPendingCommandListExecutes = {};
            eastl::vector<eastl::pair<IDXGISwapChain3*, UINT>> mPendingSwapPresents = {};
        private:
            CommandQueueInfo mInfo = {};
            ComPtr<ID3D12CommandQueue> mCommandQueue = {};
        };
    }
}