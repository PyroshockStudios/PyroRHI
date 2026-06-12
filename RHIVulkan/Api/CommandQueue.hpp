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
#include <PyroRHI/Common/AtomicMap.hpp>
#include <RHIVulkan/Core.hpp>
#include <mutex>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanFence;
        class VulkanDevice;
        class VulkanCommandBuffer;
        struct CommandBufferPool;
        class VulkanCommandQueue : public ICommandQueue, DeleteCopy, DeleteMove {
        public:
            VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info);
            ~VulkanCommandQueue();

            ICommandBuffer* GetCommandBuffer(const CommandBufferInfo& info) override;
            void WaitIdle() override;
            const CommandQueueInfo& Info() const override {
                return mInfo;
            }
            f64 GetTimestampTickPeriodNs() const override;
            PYRO_NODISCARD VkQueue GetVkQueue() {
                return mQueue;
            }
            PYRO_NODISCARD u32 GetQueueFamily() {
                return mQueueFamily;
            }
            PYRO_NODISCARD CommandBufferPool* GetCommandBufferPool() {
                return mCommandBufferPool;
            }

            PYRO_NODISCARD VulkanFence* GetGpuTimeline() {
                return gpuTimeline;
            }
            PYRO_NODISCARD u64 GetCpuTimelineValue() {
                return cpuTimeline.load();
            }

            PYRO_NODISCARD u64 IncGetCpuTimelineValue() {
                return cpuTimeline.add_fetch(1);
            }

            PYRO_NODISCARD std::lock_guard<std::mutex> AcquireAccess() {
                return std::lock_guard(mSynchMutex);
            }
            void IncrementOpenCommands();
            void DecrementOpenCommands();

            PYRO_NODISCARD bool HasOpenCommands() const {
                return mOpenCommandBuffers.load() > 0;
            }
        private:
            VulkanDevice* mDevice;
            VkQueue mQueue;
            u32 mQueueFamily;
            CommandBufferPool* mCommandBufferPool = {};

            CommandQueueInfo mInfo{};

            eastl::atomic<u64> cpuTimeline = {};
            VulkanFence* gpuTimeline = {};
            std::mutex mSynchMutex;
            eastl::atomic<u32> mOpenCommandBuffers = 0;
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios