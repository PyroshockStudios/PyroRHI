#pragma once
#include <RHIVulkan/Core.hpp>
#include <PyroRHI/Api/IFence.hpp>
#include <PyroRHI/Api/Semaphore.hpp>
#include <PyroRHI/Api/Sync.hpp>

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