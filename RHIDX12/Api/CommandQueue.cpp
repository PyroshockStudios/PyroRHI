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
#include <RHIDX12/D3DContext.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DCommandQueue::D3DCommandQueue(D3DDevice* device, CommandQueueInfo&& info, ComPtr<ID3D12CommandQueue>&& queue)
            : mDevice(device), mInfo(eastl::move(info)), mCommandQueue(eastl::move(queue)) {
            D3DSetDebugName(mCommandQueue, mInfo.name.c_str());
            CheckD3DResult(mDevice->InternalDevice() ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mCommandBufferTracker)));
        }
        D3DCommandQueue::~D3DCommandQueue() {
            for (auto& [cmb, fenceVal] : mPooledCommandBuffers) {
                delete cmb;
            }
        }
        ICommandBuffer* D3DCommandQueue::GetCommandBuffer(const CommandBufferInfo& info) {
            D3DCommandBuffer* commands = nullptr;

            UINT64 completedVal = mCommandBufferTracker->GetCompletedValue();
            for (i32 i = 0; i < mPooledCommandBuffers.size(); ++i) {
                auto& [cmb, fenceVal] = mPooledCommandBuffers[i];
                if (fenceVal > completedVal) {
                    commands = cmb;
                    mPooledCommandBuffers.erase(mPooledCommandBuffers.begin() + i);
                    break;
                }
            }
            if (!commands) {
                ComPtr<ID3D12CommandAllocator> commandAllocator = {};
                ComPtr<ID3D12GraphicsCommandList> commandList = {};

                D3D12_COMMAND_LIST_TYPE type = mCommandQueue->GetDesc().Type;
                CheckD3DResult(mDevice->InternalDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
                D3DSetDebugName(commandAllocator, (info.name + " Allocator").c_str());
                CheckD3DResult(mDevice->InternalDevice()->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
                commands = new D3DCommandBuffer(mDevice, eastl::move(commandList), eastl::move(commandAllocator));
                commands->queueFlags = mInfo.flags;
            }
            D3DSetDebugName(commands->GetCommands(), info.name.c_str());

            if (commands->bUsedBefore) {
                commands->Reset();
            } else {
                commands->bUsedBefore = true;
            }
            eastl::array<ID3D12DescriptorHeap* const, 2u> descriptorHeaps{
                mDevice->mDefaultUAVDescriptorTable.mHeap.Get(),
                mDevice->ResourcePool().mSamplerHeap.InternalHeap()
            };

            commands->GetCommands()->SetDescriptorHeaps(static_cast<UINT>(descriptorHeaps.size()), descriptorHeaps.data());
            if (mInfo.flags & CommandQueueFlagBits::COMPUTE) {
                commands->GetCommands()->SetComputeRootSignature(mDevice->mRootSignature.Get());

                commands->GetCommands()->SetComputeRootDescriptorTable(14, mDevice->mDefaultUAVDescriptorTable.gpuDescriptor);
                commands->GetCommands()->SetComputeRootDescriptorTable(15, mDevice->ResourcePool().mSamplerHeap.DeviceHandle());
            }
            if (mInfo.flags & CommandQueueFlagBits::GRAPHICS) {
                commands->GetCommands()->SetGraphicsRootSignature(mDevice->mRootSignature.Get());

                commands->GetCommands()->SetGraphicsRoot32BitConstant(17, 0, 0);

                commands->GetCommands()->SetGraphicsRootDescriptorTable(14, mDevice->mDefaultUAVDescriptorTable.gpuDescriptor);
                commands->GetCommands()->SetGraphicsRootDescriptorTable(15, mDevice->ResourcePool().mSamplerHeap.DeviceHandle());
            }

            gDx12Context->FlushDebugMessages();

            return commands;
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
            CheckD3DResult(mCommandQueue->GetDevice(IID_PPV_ARGS(&device)));

            CheckD3DResult(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

            HANDLE eventHandle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
            if (!eventHandle)
                return;

            UINT64 fenceValue = 1;
            CheckD3DResult(mCommandQueue->Signal(fence.Get(), fenceValue));
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
            return 1e9 / static_cast<f64>(freq);
        }
        void D3DCommandQueue::SignalCommandBufferFences() {
            mCommandQueue->Signal(mCommandBufferTracker.Get(), mCurrentCommandBufferFenceValue++);
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios