#include "CommandQueue.hpp"

#include <RHIVulkan/Api/CommandBuffer.hpp>
#include <RHIVulkan/Api/SwapChain.hpp>

#include <libassert/assert.hpp>


namespace PyroshockStudios::RHIVulkan {

    VulkanCommandQueue::VulkanCommandQueue(VulkanDevice* device, VkQueue queue, u32 family, const CommandQueueInfo& info)
        : mInfo(info), mDevice(device), mQueue(queue), mQueueFamily(family) {
    }
    VulkanCommandQueue::~VulkanCommandQueue() {
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
}