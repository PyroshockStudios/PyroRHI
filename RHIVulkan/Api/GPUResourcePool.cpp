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

#include "GPUResourcePool.hpp"
#include <PyroRHI/Api/ICommandBuffer.hpp>
#include <format>
#include <libassert/assert.hpp>

#define STORAGE_BUFFER_BINDING 0 /*SRV/UAV buffer*/
#define SAMPLED_IMAGE_BINDING 1  /*SRV texture*/
#define SAMPLER_BINDING 2        /*Sampler State*/

namespace PyroshockStudios {
    namespace RHIVulkan {
        VkResult GPUShaderResourceTable::Initialize(u32 maxBuffersViews, u32 maxImageViews, u32 maxSamplers,
            VkDevice device, const VkAllocationCallbacks* allocator, VkBuffer deviceAddressBuffer,
            PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT) {
            mBufferSlots.mMaxResources = maxBuffersViews;
            mImageSlots.mMaxResources = maxImageViews;
            mResourceViewSlots.mMaxResources = eastl::max(maxBuffersViews, maxImageViews);
            mSamplerSlots.mMaxResources = maxSamplers;

            const VkDescriptorPoolSize storageBufferDescriptorPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = mBufferSlots.mMaxResources + 1,
            };

            const VkDescriptorPoolSize uniformBufferDescriptorPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 16,
            };

            const VkDescriptorPoolSize storageImageDescriptorPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = mImageSlots.mMaxResources + 1,
            };

            const VkDescriptorPoolSize sampledImageDescriptorPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = mImageSlots.mMaxResources + 1,
            };

            const VkDescriptorPoolSize samplerDescriptorPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = mSamplerSlots.mMaxResources,
            };

            const eastl::array<const VkDescriptorPoolSize, 5> poolSizes = {
                storageBufferDescriptorPoolSize,
                uniformBufferDescriptorPoolSize,
                storageImageDescriptorPoolSize,
                sampledImageDescriptorPoolSize,
                samplerDescriptorPoolSize,
            };

            const VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                .maxSets = 1,
                .poolSizeCount = static_cast<u32>(poolSizes.size()),
                .pPoolSizes = poolSizes.data(),
            };

            VkResult result = vkCreateDescriptorPool(device, &vkDescriptorPoolCreateInfo, allocator, &mDescriptorPool);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const char* descriptorPoolName = "Mega Descriptor Pool";
                const VkDebugUtilsObjectNameInfoEXT descriptorPoolNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                    .objectHandle = eastl::bit_cast<uint64_t>(mDescriptorPool),
                    .pObjectName = descriptorPoolName,
                };
                vkSetDebugUtilsObjectNameEXT(device, &descriptorPoolNameInfo);
            }


            CreateBindlessDescriptorSetLayout(device, allocator);
            CreatePushDescriptorSetLayout(device, allocator);

            VkDescriptorSetLayout setLayouts[2]{
                mBindlessDescriptorSetLayout,
                mPushDescriptorSetLayout
            };

            const VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = mDescriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &mBindlessDescriptorSetLayout,
            };

            result = vkAllocateDescriptorSets(device, &vkDescriptorSetAllocateInfo, &mBindlessDescriptorSet);
            CheckVkResult(result);
            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const char* name = "Bindless Descriptor Set";
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET,
                    .objectHandle = eastl::bit_cast<uint64_t>(mBindlessDescriptorSet),
                    .pObjectName = name,
                };
                vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
            }

            const VkPushConstantRange vkPushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = 0,
                .size = Limits::MAX_PUSH_CONSTANT_SIZE,
            };
            VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .setLayoutCount = PYRO_ARRAY_SIZE(setLayouts),
                .pSetLayouts = setLayouts,
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &vkPushConstantRange,
            };

            vkCreatePipelineLayout(device, &vkPipelineLayoutCreateInfo, allocator, &mPipelineLayout);
            CheckVkResult(result);
            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const char* name = "Universal Pipeline Layout";
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    .objectHandle = eastl::bit_cast<uint64_t>(mPipelineLayout),
                    .pObjectName = name,
                };
                vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
            }
            return result;
        }

        void GPUShaderResourceTable::Cleanup(VkDevice device, const VkAllocationCallbacks* allocator) {
            [[maybe_unused]] auto printRemaining = [&](std::string prefix, auto& pages) {
                std::string ret{ prefix + "\nthis can happen due to not waiting for the gpu to finish executing, as Shockwave defers destruction. List of survivors:\n" };
                for (auto& page : pages) {
                    if (page) {
                        for (auto& slot : *page) {
                            bool handleInvalid = {};
                            if constexpr (std::is_same_v<decltype(slot.first), ImplBufferSlot>) {
                                handleInvalid = slot.first.vkBuffer == VK_NULL_HANDLE;
                            }
                            if constexpr (std::is_same_v<decltype(slot.first), ImplImageSlot>) {
                                handleInvalid = slot.first.vkImage == VK_NULL_HANDLE;
                            }
                            if constexpr (std::is_same_v<decltype(slot.first), ImplSamplerSlot>) {
                                handleInvalid = slot.first.vkSampler == VK_NULL_HANDLE;
                            }
                            if (!handleInvalid) {
                                ret += "debug name : \"" + std::string(slot.first.info.name.c_str()) + "\"";
                                ret += "\n";
                            }
                        }
                    }
                }
                return ret;
            };

            if (mBufferSlots.mFreeIndexStack.size() != mBufferSlots.mNextIndex - 1) {
                printRemaining("Detected leaked buffers; not all buffers have been destroyed before destroying the device;", mBufferSlots.pages);
            }
            if (mImageSlots.mFreeIndexStack.size() != mImageSlots.mNextIndex - 1) {
                printRemaining("Detected leaked images; not all images have been destroyed before destroying the device;", mImageSlots.pages);
            }
            if (mSamplerSlots.mFreeIndexStack.size() != mSamplerSlots.mNextIndex - 1) {
                printRemaining("Detected leaked samplers; not all samplers have been destroyed before destroying the device;", mSamplerSlots.pages);
            }

            vkDestroyPipelineLayout(device, mPipelineLayout, allocator);

            vkDestroyDescriptorSetLayout(device, mBindlessDescriptorSetLayout, allocator);
            vkDestroyDescriptorSetLayout(device, mPushDescriptorSetLayout, allocator);
            vkResetDescriptorPool(device, mDescriptorPool, {});
            vkDestroyDescriptorPool(device, mDescriptorPool, allocator);
        }

        void GPUShaderResourceTable::WriteDescriptorSetSampler(VkDevice vkDevice, VkDescriptorSet vkDescriptorSet, VkSampler vkSampler, u32 index) {
            const VkDescriptorImageInfo vkDescriptorImageInfo = {
                .sampler = vkSampler,
                .imageView = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };

            const VkWriteDescriptorSet vkWriteDescriptorSetStorage = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = vkDescriptorSet,
                .dstBinding = SAMPLER_BINDING,
                .dstArrayElement = index,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .pImageInfo = &vkDescriptorImageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };

            vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSetStorage, 0, nullptr);
        }

        void GPUShaderResourceTable::WriteDescriptorSetBuffer(VkDevice vkDevice, VkDescriptorSet vkDescriptorSet, VkBuffer vkBuffer, VkDeviceSize offset, VkDeviceSize range, u32 index) {
            const VkDescriptorBufferInfo vkDescriptorImageInfo = {
                .buffer = vkBuffer,
                .offset = offset,
                .range = range,
            };

            const VkWriteDescriptorSet vkWriteDescriptorSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = vkDescriptorSet,
                .dstBinding = STORAGE_BUFFER_BINDING,
                .dstArrayElement = index,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &vkDescriptorImageInfo,
                .pTexelBufferView = nullptr,
            };

            vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, nullptr);
        }

        void GPUShaderResourceTable::WriteDescriptorSetImageView(VkDevice vkDevice, VkDescriptorSet vkDescriptorSet, VkImageView vkImageView, ImageUsageFlags usage, u32 index) {
            u32 descriptorSetWriteCount = 0;
            eastl::array<VkWriteDescriptorSet, 2> desciptorSetWrites = {};

            const VkDescriptorImageInfo vkDescriptorImageInfo = {
                .sampler = VK_NULL_HANDLE,
                .imageView = vkImageView,
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            };

            const VkDescriptorImageInfo vkDescriptorImageInfoSampled = {
                .sampler = VK_NULL_HANDLE,
                .imageView = vkImageView,
                .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            };

            const VkWriteDescriptorSet vkWriteDescriptorSetSampled = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = vkDescriptorSet,
                .dstBinding = SAMPLED_IMAGE_BINDING,
                .dstArrayElement = index,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .pImageInfo = &vkDescriptorImageInfoSampled,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };

            if ((usage & ImageUsageFlagBits::SHADER_RESOURCE) != ImageUsageFlagBits::NONE) {
                desciptorSetWrites.at(descriptorSetWriteCount++) = vkWriteDescriptorSetSampled;
            }

            vkUpdateDescriptorSets(vkDevice, descriptorSetWriteCount, desciptorSetWrites.data(), 0, nullptr);
        }



        void GPUShaderResourceTable::CreateBindlessDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator) {
            const VkDescriptorSetLayoutBinding bufferDescriptorSetLayoutBinding = {
                .binding = STORAGE_BUFFER_BINDING,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = static_cast<u32>(mBufferSlots.mMaxResources),
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            };

            const VkDescriptorSetLayoutBinding sampledImageDescriptorSetLayoutBinding = {
                .binding = SAMPLED_IMAGE_BINDING,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = static_cast<u32>(mImageSlots.mMaxResources),
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            };

            const VkDescriptorSetLayoutBinding samplerDescriptorSetLayoutBinding = {
                .binding = SAMPLER_BINDING,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = static_cast<u32>(mSamplerSlots.mMaxResources),
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            };

            const eastl::array<VkDescriptorSetLayoutBinding, 3> descriptorSetLayoutBindings = {
                bufferDescriptorSetLayoutBinding,
                sampledImageDescriptorSetLayoutBinding,
                samplerDescriptorSetLayoutBinding
            };

            const eastl::array<const VkDescriptorBindingFlags, 3> vkDescriptorBindingFlags = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            };

            const VkDescriptorSetLayoutBindingFlagsCreateInfo vkDescriptorSetLayoutBindingFlagsCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .pNext = nullptr,
                .bindingCount = static_cast<u32>(vkDescriptorBindingFlags.size()),
                .pBindingFlags = vkDescriptorBindingFlags.data(),
            };

            const VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &vkDescriptorSetLayoutBindingFlagsCreateInfo,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                .bindingCount = static_cast<u32>(descriptorSetLayoutBindings.size()),
                .pBindings = descriptorSetLayoutBindings.data(),
            };

            VkResult result = vkCreateDescriptorSetLayout(device, &vkDescriptorSetLayoutCreateInfo, allocator, &mBindlessDescriptorSetLayout);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const char* name = "Bindless Sescriptor Set Layout";
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    .objectHandle = eastl::bit_cast<uint64_t>(mBindlessDescriptorSetLayout),
                    .pObjectName = name,
                };
                result = vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
                CheckVkResult(result);
            }
        }

        void GPUShaderResourceTable::CreatePushDescriptorSetLayout(VkDevice device, const VkAllocationCallbacks* allocator) {
            eastl::array<VkDescriptorSetLayoutBinding, 32> descriptorSetLayoutBindings;

            // 0-7 = uniform
            for (u32 i = 0; i < Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS; ++i) {
                descriptorSetLayoutBindings[i] = {
                    .binding = i,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                };
            }

            // 8-19 = storage buffer
            for (u32 i = Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS; i < Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS; ++i) {
                descriptorSetLayoutBindings[i] = {
                    .binding = i,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                };
            }

            // 20-31 = storage image
            for (u32 i = Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS; i < Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS + 2 * Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS; ++i) {
                descriptorSetLayoutBindings[i] = {
                    .binding = i,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                };
            }

            const VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
                .bindingCount = static_cast<u32>(descriptorSetLayoutBindings.size()),
                .pBindings = descriptorSetLayoutBindings.data()
            };

            VkResult result = vkCreateDescriptorSetLayout(device, &vkDescriptorSetLayoutCreateInfo,
                allocator, &mPushDescriptorSetLayout);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const char* name = "Push Descriptor Set Layout";
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    .objectHandle = eastl::bit_cast<uint64_t>(mPushDescriptorSetLayout),
                    .pObjectName = name,
                };
                result = vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
                CheckVkResult(result);
            }
        }
    }
}