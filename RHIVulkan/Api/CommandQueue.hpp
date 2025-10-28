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

#include <PyroRHI/Api/ICommandQueue.hpp>
#include <RHIVulkan/Core.hpp>
#include <EASTL/atomic.h>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanFence;
        class VulkanDevice;
        class VulkanCommandBuffer;
        class CommandBufferPool;
        class VulkanCommandQueue : public ICommandQueue, DeleteCopy, DeleteMove {
        public:
            VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info);
            ~VulkanCommandQueue();

            ICommandBuffer* GetCommandBuffer(const CommandBufferInfo& info) override;
            void SubmitCommandBuffer(ICommandBuffer*& commandBuffer) override;
            void SubmitSwapChain(ISwapChain* swapChain) override;
            void WaitIdle() override;
            const CommandQueueInfo& Info() const override {
                return mInfo;
            }
            f64 GetTimestampTickPeriodNs() const override;
            VkQueue GetVkQueue() {
                return mQueue;
            }
            u32 GetQueueFamily() {
                return mQueueFamily;
            }
            CommandBufferPool* GetCommandBufferPool() {
                return mCommandBufferPool;
            }
            eastl::vector<VulkanCommandBuffer*>& RefSubmittedCommandBuffers() {
                return mCommandBuffers;
            }
            eastl::vector<VkSwapchainKHR>& RefSubmittedSwapChains() {
                return mSwapChains;
            }
            eastl::vector<u32>& RefSubmittedSwapChainImageIndices() {
                return mImageIndices;
            }
            eastl::vector<VkSemaphore>& RefSubmittedSwapAcquireSemaphores() {
                return mSwapChainAcquireSemaphores;
            }

            bool mbPendingSwapPresent = false;

            VulkanFence* GetGpuTimeline() {
                return gpuTimeline;
            }
            eastl::atomic<u64>& RefCpuTimelineValue() {
                return cpuTimeline;
            }

        private:
            VulkanDevice* mDevice;
            VkQueue mQueue;
            u32 mQueueFamily;
            eastl::vector<VulkanCommandBuffer*> mCommandBuffers{};
            eastl::vector<VkSwapchainKHR> mSwapChains{};
            eastl::vector<VkSemaphore> mSwapChainAcquireSemaphores{};
            eastl::vector<u32> mImageIndices{};

            CommandBufferPool* mCommandBufferPool = {};

            CommandQueueInfo mInfo{};

            eastl::atomic<u64> cpuTimeline = {};
            VulkanFence* gpuTimeline = {};
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios