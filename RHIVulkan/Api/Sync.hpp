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
#include <PyroRHI/Api/IFence.hpp>
#include <PyroRHI/Api/Semaphore.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;

        class VulkanSemaphore : DeleteCopy, DeleteMove {
        public:
            VulkanSemaphore(VkSemaphore semaphore, VulkanDevice* device, const SemaphoreInfo& info);
            ~VulkanSemaphore();
            const SemaphoreInfo& Info() const {
                return mInfo;
            }

            VkSemaphore GetVkSemaphore() {
                return mSemaphore;
            }

        private:
            VkSemaphore mSemaphore;
            VulkanDevice* mDevice;
            SemaphoreInfo mInfo;
        };

        class VulkanFence : public IFence, DeleteCopy, DeleteMove {
        public:
            VulkanFence(VkSemaphore semaphore, VulkanDevice* device, const FenceInfo& info);
            ~VulkanFence();

            const FenceInfo& Info() const override {
                return mInfo;
            }

            u64 Value() const override;
            void SetValue(u64 value) override;
            bool WaitForValue(u64 value, u64 timeoutNs = ~u64(0)) override;

            VkSemaphore GetVkSemaphore() {
                return mSemaphore;
            }

        private:
            VkSemaphore mSemaphore;
            VulkanDevice* mDevice;
            FenceInfo mInfo;
        };

    } // namespace RHIVulkan
} // namespace PyroshockStudios