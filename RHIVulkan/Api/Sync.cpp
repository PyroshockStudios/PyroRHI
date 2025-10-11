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

#include "Sync.hpp"
#include "Device.hpp"

#include <RHIVulkan/VkContext.hpp>


namespace PyroshockStudios {
    namespace RHIVulkan {
        VulkanSemaphore::VulkanSemaphore(VkSemaphore semaphore, VulkanDevice* device, const SemaphoreInfo& info)
            : mSemaphore(semaphore), mDevice(device), mInfo(info) {
        }
        VulkanSemaphore::~VulkanSemaphore() {
            vkDestroySemaphore(mDevice->GetVkDevice(), mSemaphore, mDevice->Context()->GetVkAllocator());
        }
        VulkanFence::VulkanFence(VkSemaphore semaphore, VulkanDevice* device, const FenceInfo& info)
            : mSemaphore(semaphore), mDevice(device), mInfo(info) {
        }
        VulkanFence::~VulkanFence() {
            vkDestroySemaphore(mDevice->GetVkDevice(), mSemaphore, mDevice->Context()->GetVkAllocator());
        }
        u64 VulkanFence::Value() const {
            u64 value = 0;
            VkResult result = vkGetSemaphoreCounterValue(mDevice->GetVkDevice(), mSemaphore, &value);
            CheckVkResult(result);
            return value;
        }
        void VulkanFence::SetValue(u64 value) {
            VkSemaphoreSignalInfo const signalInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                .pNext = nullptr,
                .semaphore = mSemaphore,
                .value = value,
            };
            VkResult result = vkSignalSemaphore(mDevice->GetVkDevice(), &signalInfo);
            CheckVkResult(result);
        }
        bool VulkanFence::WaitForValue(u64 value, u64 timeoutNs) {
            VkSemaphoreWaitInfo waitInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                .pNext = nullptr,
                .flags = {},
                .semaphoreCount = 1,
                .pSemaphores = &mSemaphore,
                .pValues = &value,
            };

            return vkWaitSemaphores(mDevice->GetVkDevice(), &waitInfo, timeoutNs) == VK_SUCCESS;
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios