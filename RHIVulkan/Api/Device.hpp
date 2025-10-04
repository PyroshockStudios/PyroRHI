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
#include <EASTL/array.h>
#include <EASTL/atomic.h>
#include <EASTL/deque.h>
#include <EASTL/stack.h>
#include <EASTL/unordered_set.h>

#include <RHIVulkan/Api/CommandBuffer.hpp>
#include <RHIVulkan/Api/GPUResourcePool.hpp>
#include <RHIVulkan/Core.hpp>
#include <PyroRHI/Api/IDevice.hpp>


namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanContext;
        class VulkanSwapChain;
        class VulkanCommandQueue;

        struct VulkanSwapChainSupportInfo {
            VkSurfaceCapabilitiesKHR capabilities;
            eastl::vector<VkSurfaceFormatKHR> formats;
            eastl::vector<VkPresentModeKHR> presentModes;
        };

        struct CommandListZombie {
            VkCommandBuffer vkCmdBuffer = {};
            VkCommandPool vkCmdPool = {};
            eastl::unique_ptr<CommandBufferZombieInfo> zombies = {};
        };

        struct VulkanDeviceCapabilities {
            bool bVK_EXT_line_rasterization = false;
            bool bVK_EXT_buffer_device_address = false;
        };

        class VulkanDevice : public IDevice, DeleteCopy, DeleteMove {
        public:
            VulkanDevice(VulkanContext* context, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& features);
            virtual ~VulkanDevice() override;

            bool IsMemoryBlockValid(MemoryBlock handle) const override;
            bool IsBufferValid(Buffer handle) const override;
            bool IsImageValid(Image handle) const override;
            bool IsShaderResourceValid(ShaderResourceId id) const override;
            bool IsUnorderedAccessValid(UnorderedAccessId id) const override;
            bool IsSamplerValid(SamplerId id) const override;

            const MemoryBlockInfo& GetMemoryBlockInfo(MemoryBlock memory) const override;
            const BufferInfo& GetBufferInfo(Buffer buffer) const override;
            const ImageInfo& GetImageInfo(Image image) const override;
            const GPUResourceInfo& GetShaderResourceInfo(ShaderResourceId id) const override;
            const GPUResourceInfo& GetUnorderedAccessInfo(UnorderedAccessId id) const override;
            const SamplerInfo& GetSamplerInfo(SamplerId id) const override;
            const RenderTargetInfo& GetRenderTargetInfo(RenderTarget renderTarget) const       override;
            const RasterPipelineInfo& GetRasterPipelineInfo(RasterPipeline pipeline) const     override;
            const ComputePipelineInfo& GetComputePipelineInfo(ComputePipeline pipeline) const  override;
            const SemaphoreInfo& GetSemaphoreInfo(Semaphore semaphore) const override;

            DeviceAddress BufferDeviceAddress(Buffer buffer) const override;
            u8* BufferHostAddress(Buffer buffer) const override;

            DeviceSize ImageSizeRequirements(Image image) const override;
            u32 ImageSubresourceRowPitch(Image image, ImageSlice slice, u32 rowWidth) const override;

            MemoryBlock CreateMemoryBlock(const MemoryBlockInfo& info) override;
            Buffer CreateBuffer(const BufferInfo& info) override;
            Image CreateImage(const ImageInfo& info) override;
            ShaderResourceId CreateShaderResource(const GPUResourceInfo& info) override;
            UnorderedAccessId CreateUnorderedAccess(const GPUResourceInfo& info) override;
            SamplerId CreateSampler(const SamplerInfo& info) override;

            RenderTarget CreateRenderTarget(const RenderTargetInfo& info) override;
            RasterPipeline CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) override;
            ComputePipeline CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) override;
            ISwapChain* CreateSwapChain(const SwapChainInfo& info) override;
            Semaphore CreateSemaphore(const SemaphoreInfo& info) override;
            IFence* CreateFence(const FenceInfo& info) override;
            ITimestampQueryPool* CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) override;

            ICommandBuffer* GetCommandBuffer(const CommandBufferInfo& info) override;

            virtual void DestroyMemoryBlock(MemoryBlock& memory) override;
            virtual void DestroyBuffer(Buffer& buffer) override;
            virtual void DestroyImage(Image& image) override;
            virtual void DestroyShaderResource(ShaderResourceId& srv) override;
            virtual void DestroyUnorderedAccess(UnorderedAccessId& uav) override;
            virtual void DestroySampler(SamplerId& sampler) override;

            virtual void DestroyRenderTarget(RenderTarget& renderTarget) override;
            virtual void DestroyRasterPipeline(RasterPipeline& pipeline) override;
            virtual void DestroyComputePipeline(ComputePipeline& pipeline) override;
            virtual void DestroySwapChain(ISwapChain*& swapChain) override;
            virtual void DestroySemaphore(Semaphore& semaphore) override;
            virtual void DestroyFence(IFence*& fence) override;
            virtual void DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool) override;

            eastl::optional<Format> PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) override;

            void WaitIdle() override;
            void SubmitQueue(const CommandQueueSubmitInfo& info) override;
            void PresentQueue(const CommandQueuePresentInfo& info) override;

            const VkPhysicalDeviceProperties& GetVkPhysicalDeviceProperties() {
                return mPhysicalDeviceProperties;
            }
        public:
            Image NewSwapChainImage(VkImage swapchainImage, VkFormat format, u32 index, ImageUsageFlags usage, const ImageInfo& imageInfo);

            VulkanSwapChainSupportInfo GetSwapChainSupport(VkSurfaceKHR surface) const;

            uint32_t FindMemoryTypeIndex(
                uint32_t memoryTypeBits,                  
                VkMemoryPropertyFlags requiredProperties);

            eastl::span<ICommandQueue*> GetCommandQueues() override {
                return mCommandQueues;
            }
            ICommandQueue* GetPresentQueue() override {
                return mPresentQueue;
            }

            const DeviceInfo& GetInfo() override;
            const DevicePropertiesInfo& GetProperties() override;

            VulkanContext* Context() {
                return mContext;
            }
            VkDevice GetVkDevice() {
                return mDevice;
            }
            VkPhysicalDevice GetVkPhysicalDevice() {
                return mPhysicalDevice;
            }
            GPUShaderResourceTable& GetResourceTable() {
                return mResourceTable;
            }

            ImplVmaVirtualBlockSlot& Slot(MemoryBlock block);
            ImplBufferSlot& Slot(Buffer buffer);
            ImplImageSlot& Slot(Image image);
            ImplResourceViewSlot& Slot(ShaderResourceId id);
            ImplResourceViewSlot& Slot(UnorderedAccessId id);
            ImplSamplerSlot& Slot(SamplerId id);

            const ImplVmaVirtualBlockSlot& Slot(MemoryBlock block) const;
            const ImplBufferSlot& Slot(Buffer buffer) const;
            const ImplImageSlot& Slot(Image image) const;
            const ImplResourceViewSlot& Slot(ShaderResourceId id) const;
            const ImplResourceViewSlot& Slot(UnorderedAccessId id) const;
            const ImplSamplerSlot& Slot(SamplerId id) const;

        public:
            eastl::atomic<u64> mMainQueueCpuTimeline = {};

            eastl::deque<eastl::pair<u64, CommandListZombie>> mMainQueueCommandListZombies = {};
            eastl::deque<eastl::pair<u64, ZombieDeleter>> mMainQueueZombies = {};

            VulkanDeviceCapabilities mVulkanCaps = {};
        private:
            GPUResourceId CreateImageView(const GPUResourceInfo& info, bool uav);
            GPUResourceId CreateBufferView(const GPUResourceInfo& info);
            void CollectGarbage();


            CommandBufferPool* mCommandBufferPool = {};

            IFence* mMainQueueGpuFence = {};

            DeviceInfo mInfo = {};
            DevicePropertiesInfo mProperties = {};

            VulkanContext* mContext;
            VkPhysicalDeviceProperties mPhysicalDeviceProperties;
            VkPhysicalDevice mPhysicalDevice;
            VkDevice mDevice;

            VmaAllocator mVmaAllocator = {};

            VolkDeviceTable mDeviceTable = {};

            GPUShaderResourceTable mResourceTable = {};

            eastl::vector<ICommandQueue*> mCommandQueues = {};
            ICommandQueue* mPresentQueue = {};
            eastl::vector<u32> mUniqueCommandQueueFamilies = {};

            eastl::unordered_set<VulkanSwapChain*> mActiveSwapChains = {};
            eastl::unordered_set<VulkanCommandBuffer*> mAllocatedCommandBuffers = {};

            usize mNumAllocatedCommandPools = 0;
        };
    }
}