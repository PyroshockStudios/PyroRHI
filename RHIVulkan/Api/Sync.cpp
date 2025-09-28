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
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
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