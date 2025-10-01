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

#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "SwapChain.hpp"
namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DCommandQueue::D3DCommandQueue(CommandQueueInfo&& info, ComPtr<ID3D12CommandQueue>&& queue)
            : mInfo(eastl::move(info)), mCommandQueue(eastl::move(queue)) {
            D3DSetDebugName(mCommandQueue, mInfo.name.c_str());
        }
        D3DCommandQueue::~D3DCommandQueue() {
        }
        void D3DCommandQueue::SubmitCommandBuffer(ICommandBuffer*& commandBuffer) {
            auto* d3dcmd = static_cast<D3DCommandBuffer*>(commandBuffer);
            mSubmittedCommands.emplace_back(d3dcmd);
            mPendingCommandListExecutes.emplace_back(d3dcmd->GetCommands());
            commandBuffer = nullptr;
        }
        void D3DCommandQueue::SubmitSwapChain(ISwapChain* swapChain) {
            auto* swap = static_cast<D3DSwapChain*>(swapChain);
            mPendingSwapPresents.emplace_back(swap->InternalSwapChain(), swap->mSyncInterval);
            swapChain = nullptr;
        }
        void D3DCommandQueue::WaitIdle() {
            ComPtr<ID3D12Fence> fence;
            ComPtr<ID3D12Device> device;
            mCommandQueue->GetDevice(IID_PPV_ARGS(&device));

            CheckD3DResult(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

            HANDLE eventHandle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
            if (!eventHandle)
                return;

            UINT64 fenceValue = 1;
            mCommandQueue->Signal(fence.Get(), fenceValue);
            if (fence->GetCompletedValue() < fenceValue) {
                fence->SetEventOnCompletion(fenceValue, eventHandle);
                WaitForSingleObject(eventHandle, INFINITE);
            }
            CloseHandle(eventHandle);
        }
        const CommandQueueInfo& D3DCommandQueue::Info() const {
            return mInfo;
        }
        f64 D3DCommandQueue::GetTimestampTickPeriodNs() const {
            UINT64 freq = 0;
            CheckD3DResult(mCommandQueue->GetTimestampFrequency(&freq));
            return freq;
        }
    }
}