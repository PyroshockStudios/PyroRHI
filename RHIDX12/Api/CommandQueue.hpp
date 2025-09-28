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