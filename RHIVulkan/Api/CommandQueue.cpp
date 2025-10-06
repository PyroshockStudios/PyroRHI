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

#include <RHIVulkan/Api/CommandBuffer.hpp>
#include <RHIVulkan/Api/SwapChain.hpp>

#include <libassert/assert.hpp>


namespace PyroshockStudios::RHIVulkan {

    VulkanCommandQueue::VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info)
        : mInfo(info), mDevice(device), mQueue(queue), mQueueFamily(family) {
        mCommandBufferPool = new CommandBufferPool();
    }
    VulkanCommandQueue::~VulkanCommandQueue() {
        mCommandBufferPool->Cleanup(mDevice, this);
        delete mCommandBufferPool;
    }

    ICommandBuffer* VulkanCommandQueue::GetCommandBuffer(const CommandBufferInfo& info) {
        auto [pool, buffer] = mCommandBufferPool->Get(mDevice, this);
        return new VulkanCommandBuffer(mDevice, pool, buffer, info);
    }

    void VulkanCommandQueue::SubmitCommandBuffer(ICommandBuffer*& commandBuffer) {
        mCommandBuffers.push_back(static_cast<VulkanCommandBuffer*>(commandBuffer));
        commandBuffer = nullptr;
    }
    void VulkanCommandQueue::SubmitSwapChain(ISwapChain* swapChain) {
        auto* swap = static_cast<VulkanSwapChain*>(swapChain);
        mSwapChains.emplace_back(swap->GetVkSwapChain());
        mSwapChainAcquireSemaphores.emplace_back(swap->GetCurrentImageAcquireSemaphore());
        mImageIndices.emplace_back(swap->GetCurrentImageIndex());
    }
    void VulkanCommandQueue::WaitIdle() {
        vkQueueWaitIdle(mQueue);
    }
    f64 VulkanCommandQueue::GetTimestampTickPeriodNs() const {
        return static_cast<f64>(mDevice->GetVkPhysicalDeviceProperties().limits.timestampPeriod);
    }
}