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