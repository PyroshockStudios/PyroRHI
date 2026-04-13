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

#include <cassert>

#include <EASTL/array.h>
#include <EASTL/atomic.h>
#include <mutex>
#include <shared_mutex>
// TODO?
// #include<EATHREAD / eathread_mutex.h>
#include <EASTL/bit.h>
#include <EASTL/optional.h>
#include <EASTL/unique_ptr.h>

#include <PyroRHI/Api/AccelerationStructure.hpp>
#include <PyroRHI/Api/GPUResource.hpp>
#include <RHIVulkan/Core.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanSwapChain;
        class VulkanDevice;
        struct ZombieDeleter {
            void* resource = {};
            FunctionPtr<void(VulkanDevice* device, void* resource)> deleter = {};
            eastl::vector<u64> queueTimelineSnapshot{};
        };

        struct ImplBufferSlot {
            BufferInfo info = {};
            VkBuffer vkBuffer = {};
            Union<VmaVirtualAllocation, VmaAllocation> vmaAllocation = {};
            Union<VmaVirtualAllocationInfo, VmaAllocationInfo> allocationInfo = {};
            VkDeviceAddress deviceAddress = {};
            void* hostAddress = {};
            bool zombie = {};
        };

        static inline constexpr i32 NOT_OWNED_BY_SWAPCHAIN = -1;

        struct ImplResourceViewSlot {
            GpuResourceInfo info = {};
            Union<VkDescriptorBufferInfo, VkDescriptorImageInfo> descriptor = {};
            bool zombie = {};
        };

        struct ImplImageSlot {
            ImageInfo info = {};
            VkImage vkImage = VK_NULL_HANDLE;
            Union<VmaVirtualAllocation, VmaAllocation> vmaAllocation = {};
            Union<VmaVirtualAllocationInfo, VmaAllocationInfo> allocationInfo = {};
            i32 swapchainImageIndex = NOT_OWNED_BY_SWAPCHAIN;
            VulkanSwapChain* ownedSwapchain = nullptr;
            VkImageAspectFlags aspectFlags = {}; // Inferred from format.
            bool zombie = {};
        };

        struct ImplSamplerSlot {
            SamplerInfo info = {};
            VkSampler vkSampler = VK_NULL_HANDLE;
            bool zombie = {};
        };

        struct ImplVmaVirtualBlockSlot {
            MemoryBlockInfo info = {};
            VmaVirtualBlock vmaBlock = VK_NULL_HANDLE;
            VmaAllocation vmaAllocation = VK_NULL_HANDLE;
            VmaAllocationInfo vmaAllocationInfo = {};
            VkMemoryRequirements requirements = {};
            bool zombie = {};
            // eastl::atomic<u32> debugReferences = 0;
            u32 debugReferences = 0;
        };

        struct ImplBlasSlot {
            BlasInfo info = {};
            VkAccelerationStructureKHR vkAccelerationStructure = {};
            Buffer bufferId = {};
            VkBuffer vkBuffer = {};
            u64 offset = {};
            VkDeviceAddress deviceAddress = {};
            bool ownsBuffer = {};
            bool zombie = {};
        };

        struct ImplTlasSlot {
            TlasInfo info = {};
            VkAccelerationStructureKHR vkAccelerationStructure = {};
            Buffer bufferId = {};
            VkBuffer vkBuffer = {};
            u64 offset = {};
            VkDeviceAddress deviceAddress = {};
            bool ownsBuffer = {};
            bool zombie = {};
        };

        template <typename ResourceT>
        struct GpuResourcePool {
            static constexpr inline usize MAX_RESOURCE_COUNT = 1u << 20u;
            static constexpr inline usize PAGE_BITS = 10u;
            static constexpr inline usize PAGE_SIZE = 1u << PAGE_BITS;
            static constexpr inline usize PAGE_MASK = PAGE_SIZE - 1u;
            static constexpr inline usize PAGE_COUNT = MAX_RESOURCE_COUNT / PAGE_SIZE;
            using VersionAndRefcntT = eastl::atomic<u64>;
            static constexpr inline u64 VERSION_ZOMBIE_BIT = u64(1) << u64(63);
            static constexpr inline u64 VERSION_COUNT_MASK = ~(VERSION_ZOMBIE_BIT);
            using PageT = eastl::array<eastl::pair<ResourceT, VersionAndRefcntT>, PAGE_SIZE>;

            eastl::vector<u32> mFreeIndexStack = {};
            u32 mNextIndex = 1;
            u32 mMaxResources = {};

            std::mutex mPageAllocMtx = {};
            mutable std::mutex mUseAfterFreeChecMtx = {};

            eastl::array<eastl::unique_ptr<PageT>, PAGE_COUNT> pages = {};

            void VerifyResourceId(GpuResourceId id) const {
                usize page = id.index >> PAGE_BITS;
                assert(page < pages.size() && "detected invalid resource id");
                assert(pages[page] != nullptr && "detected invalid resource id");
                assert(id.version != 0 && "detected invalid resource id");
            }

            eastl::pair<GpuResourceId, ResourceT&> NewSlot() {
                std::unique_lock useAfterFreeCheckLock{ mUseAfterFreeChecMtx };
                std::unique_lock page_alloc_lock{ mPageAllocMtx };
                u32 index;
                if (mFreeIndexStack.empty()) {
                    index = mNextIndex++;
                    assert(index < MAX_RESOURCE_COUNT && "exceeded max resource count");
                    assert(index < mMaxResources && "exceeded max resource count");
                } else {
                    index = mFreeIndexStack.back();
                    mFreeIndexStack.pop_back();
                }

                usize page = index >> PAGE_BITS;
                usize offset = index & PAGE_MASK;

                if (!pages[page]) {
                    pages[page] = eastl::make_unique<PageT>();
                    for (u32 i = 0; i < PAGE_SIZE; ++i) {
                        pages[page]->at(i).second = 0; // set all version numbers to 0 (invalid)
                    }
                }

                pages[page]->at(offset).second = eastl::max<u8>(pages[page]->at(offset).second, 1); // make sure the version is at least one

                u32 version = pages[page]->at(offset).second;
                return { GpuResourceId{ .index = index, .version = version }, pages[page]->at(offset).first };
            }

            void ReturnSlot(GpuResourceId id) {
                usize page = id.index >> PAGE_BITS;
                usize offset = id.index & PAGE_MASK;

                std::unique_lock useAfterFreeCheckLock{ mUseAfterFreeChecMtx };
                VerifyResourceId(id);
                assert(pages[page]->at(offset).second == id.version && "detected double delete for a resource id");
                std::unique_lock page_alloc_lock{ mPageAllocMtx };

                pages[page]->at(offset).second = eastl::max<u8>(pages[page]->at(offset).second + 1, 1); // the max is needed, as version = 0 is invalid

                mFreeIndexStack.push_back(id.index);
            }

            bool IsIdValid(GpuResourceId id) const {
                usize page = id.index >> PAGE_BITS;
                usize offset = id.index & PAGE_MASK;

                if (!(page < pages.size()) || !(pages[page] != nullptr) || !(id.version != 0)) {
                    return false;
                }
                u8 version = pages[page]->at(offset).second;
                if (!(version == id.version) || pages[page]->at(offset).first.zombie) {
                    return false;
                }
                return true;
            }

            ResourceT& DereferenceId(GpuResourceId id) {
                usize page = id.index >> PAGE_BITS;
                usize offset = id.index & PAGE_MASK;

                std::unique_lock useAfterFreeCheckLock{ mUseAfterFreeChecMtx };
                VerifyResourceId(id);
                u8 version = pages[page]->at(offset).second;
                assert(version == id.version && "detected use after free for a resource id");
                return pages[page]->at(offset).first;
            }

            const ResourceT& DereferenceId(GpuResourceId id) const {
                usize page = id.index >> PAGE_BITS;
                usize offset = id.index & PAGE_MASK;

                std::unique_lock useAfterFreeCheckLock{ mUseAfterFreeChecMtx };
                VerifyResourceId(id);
                u8 version = pages[page]->at(offset).second;
                assert(version == id.version && "detected use after free for a resource id");
                return pages[page]->at(offset).first;
            }
        };

        struct GPUShaderResourceTable {
            std::shared_mutex mLifetimeLock = {};
            GpuResourcePool<ImplBufferSlot> mBufferSlots = {};
            GpuResourcePool<ImplImageSlot> mImageSlots = {};

            GpuResourcePool<ImplVmaVirtualBlockSlot> mVirtualBlockSlots = {};

            GpuResourcePool<ImplResourceViewSlot> mResourceViewSlots = {};

            GpuResourcePool<ImplSamplerSlot> mSamplerSlots = {};

            GpuResourcePool<ImplBlasSlot> mBlasSlots = {};

            bool bAccelerationStructures = false;
            GpuResourcePool<ImplTlasSlot> mTlasSlots = {};

            VkDescriptorSetLayout mBindlessDescriptorSetLayout = {};
            VkDescriptorSetLayout mPushDescriptorSetLayout = {};
            VkDescriptorSet mBindlessDescriptorSet = {};
            VkDescriptorPool mDescriptorPool = {};

            VkPipelineLayout mPipelineLayout = {};

            VkResult Initialize(
                u32 maxBuffersViews,
                u32 maxImageViews,
                u32 maxSamplers,
                u32 maxMemoryBlocks,
                u32 maxAccelerationStructures, // set this to 0 to disable AS
                VkDevice device,
                const VkAllocationCallbacks* allocator,
                VkBuffer deviceAddressBuffer,
                PFN_vkSetDebugUtilsObjectNameEXT fn_VkSetDebugUtilsObjectNameEXT);

            void Cleanup(VkDevice device, const VkAllocationCallbacks* allocator);

            void WriteDescriptorSetSampler(VkDevice vkDevice, VkDescriptorSet descriptorSet, VkSampler vkSampler, u32 index);
            void WriteDescriptorSetBuffer(VkDevice vkDevice, VkDescriptorSet descriptorSet, VkBuffer vkBuffer, VkDeviceSize offset, VkDeviceSize range, u32 index);
            void WriteDescriptorSetImageView(VkDevice vkDevice, VkDescriptorSet descriptorSet, VkImageView vkImageView, ImageUsageFlags usage, u32 index);
            void WriteDescriptorSetAccelerationStructure(VkDevice vkDevice, VkDescriptorSet vkDescriptorSet, VkAccelerationStructureKHR vkAccelerationStructure, u32 index);

        private:
            void CreateBindlessDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator);
            void CreatePushDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator);
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios