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

#include <RHIVulkan/Core.hpp>
#include <PyroRHI/Api/ICommandQueue.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;
        class VulkanCommandBuffer;
        class VulkanCommandQueue : public ICommandQueue, DeleteCopy, DeleteMove {
        public:
            VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info);
            ~VulkanCommandQueue();

            void SubmitCommandBuffer(ICommandBuffer*& commandBuffer) override;
            void SubmitSwapChain(ISwapChain* swapChain) override;
            void WaitIdle() override;
            const CommandQueueInfo& Info() const override {
                return mInfo;
            }
            VkQueue GetVkQueue() {
                return mQueue;
            }
            u32 GetQueueFamily() {
                return mQueueFamily;
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

        private:
            VulkanDevice* mDevice;
            VkQueue mQueue;
            u32 mQueueFamily;
            eastl::vector<VulkanCommandBuffer*> mCommandBuffers{};
            eastl::vector<VkSwapchainKHR> mSwapChains{};
            eastl::vector<VkSemaphore> mSwapChainAcquireSemaphores{};
            eastl::vector<u32> mImageIndices{};

            CommandQueueInfo mInfo{};
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios