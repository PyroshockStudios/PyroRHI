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
#include <RHIVulkan/Api/Sync.hpp>

#include <libassert/assert.hpp>


namespace PyroshockStudios::RHIVulkan {

    VulkanCommandQueue::VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info)
        : mInfo(info), mDevice(device), mQueue(queue), mQueueFamily(family) {
        mCommandBufferPool = new CommandBufferPool();
        cpuTimeline = 0;
        gpuTimeline = static_cast<VulkanFence*>(device->CreateFence({ .initialValue = cpuTimeline, .name = info.name + " (Internal GPU Timeline)" }));
    }
    VulkanCommandQueue::~VulkanCommandQueue() {
        mDevice->DestroyImmediately(gpuTimeline);
        mCommandBufferPool->Cleanup(mDevice, this);
        delete mCommandBufferPool;
    }

    ICommandBuffer* VulkanCommandQueue::GetCommandBuffer(const CommandBufferInfo& info) {
        {
            auto lock = mDevice->AcquireQueueAccess();
            IncrementOpenCommands();
        }
        auto [pool, buffer] = mCommandBufferPool->Get(mDevice, this);
        return new VulkanCommandBuffer(mDevice, this, pool, buffer, info);
    }
    void VulkanCommandQueue::WaitIdle() {
        auto glock = mDevice->AcquireQueueAccess();
        auto qlock = AcquireAccess(); // any queue operations must be synchronised!
        CheckVkResult(vkQueueWaitIdle(mQueue), "Failed to idle command queue!");
    }
    f64 VulkanCommandQueue::GetTimestampTickPeriodNs() const {
        return static_cast<f64>(mDevice->GetVkPhysicalDeviceProperties().limits.timestampPeriod);
    }
    void VulkanCommandQueue::IncrementOpenCommands() {
        mOpenCommandBuffers.add_fetch(1);
    }
    void VulkanCommandQueue::DecrementOpenCommands() {
        ASSERT(mOpenCommandBuffers.fetch_sub(1) > 0, "Trying to decrement non-existing commands!");
    }
} // namespace PyroshockStudios::RHIVulkan