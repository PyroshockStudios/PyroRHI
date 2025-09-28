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
    }
}