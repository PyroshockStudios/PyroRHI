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

#include "Device.hpp"

#include "CommandBuffer.hpp"
#include "CommandQueue.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "RenderTarget.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"

#include <PyroCommon/Logger.hpp>

#include <EASTL/numeric_limits.h>
#include <EASTL/vector.h>

#include <RHIVulkan/VkContext.hpp>

#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        PYRO_FORCEINLINE static constexpr VkImageViewType ToVkImageViewType(ImageViewType type) { return static_cast<VkImageViewType>(type); }

        VulkanDevice::VulkanDevice(VulkanContext* context, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& features)
            : mContext(context), mPhysicalDevice(physicalDevice) {
            Logger::Trace(gVulkanSink, "Creating Vulkan Device");

            VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
                .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
                .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
                .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
                .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
                .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
                .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
                .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
                .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
                .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
                .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
                .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
                .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
                .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
                .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
                .descriptorBindingPartiallyBound = VK_TRUE,
                .descriptorBindingVariableDescriptorCount = VK_FALSE,
                .runtimeDescriptorArray = VK_TRUE,
            };

            VkPhysicalDeviceShaderDrawParametersFeatures physicalDeviceShaderDrawParametersFeatures = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
                .pNext = reinterpret_cast<void*>(&physicalDeviceDescriptorIndexingFeatures),
                .shaderDrawParameters = VK_TRUE
            };

            VkPhysicalDeviceDynamicRenderingFeatures physicalDeviceDynamicRenderingFeatures = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
                .pNext = reinterpret_cast<void*>(&physicalDeviceShaderDrawParametersFeatures),
                .dynamicRendering = VK_TRUE,
            };

            VkPhysicalDeviceSynchronization2Features physicalDeviceSynchronization2Features = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
                .pNext = reinterpret_cast<void*>(&physicalDeviceDynamicRenderingFeatures),
                .synchronization2 = VK_TRUE,
            };

            VkPhysicalDeviceTimelineSemaphoreFeatures physicalDeviceFenceFeatures = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
                .pNext = reinterpret_cast<void*>(&physicalDeviceSynchronization2Features),
                .timelineSemaphore = VK_TRUE,
            };

            VkPhysicalDeviceScalarBlockLayoutFeatures physicalDeviceScalarBlockLayoutFeatures = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
                .pNext = reinterpret_cast<void*>(&physicalDeviceFenceFeatures),
                .scalarBlockLayout = VK_TRUE,
            };

            void* lastPhysicalDevicePnext = reinterpret_cast<void*>(&physicalDeviceScalarBlockLayoutFeatures);

            eastl::vector<const char*> extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
            };

            u32 extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            eastl::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            VkPhysicalDeviceLineRasterizationFeaturesEXT lineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT };
            VkPhysicalDeviceBufferDeviceAddressFeatures physicalDeviceBufferDeviceAddressFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
            for (VkExtensionProperties& extension : availableExtensions) {
                if (strcmp(extension.extensionName, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME) == 0) {
                    VkPhysicalDeviceFeatures2 features2{
                        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                        .pNext = &lineFeatures,
                    };
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
                    if (lineFeatures.smoothLines == VK_TRUE) {
                        lineFeatures.pNext = lastPhysicalDevicePnext;
                        lastPhysicalDevicePnext = reinterpret_cast<void*>(&lineFeatures);

                        Logger::Info(gVulkanSink, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME " with 'smoothLines' is supported on this device.");
                        mVulkanCaps.bVK_EXT_line_rasterization = true;
                        extensions.push_back(extension.extensionName);
                    }
                }
                if (strcmp(extension.extensionName, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0) {
                    VkPhysicalDeviceFeatures2 features2{
                        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                        .pNext = &physicalDeviceBufferDeviceAddressFeatures,
                    };
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
                    if (features2.features.shaderInt64 == VK_TRUE && physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress == VK_TRUE) {
                        physicalDeviceBufferDeviceAddressFeatures.pNext = lastPhysicalDevicePnext;
                        lastPhysicalDevicePnext = reinterpret_cast<void*>(&physicalDeviceBufferDeviceAddressFeatures);

                        Logger::Info(gVulkanSink, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME " is supported on this device.");

                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddressCaptureReplay = VK_FALSE;
                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddressMultiDevice = VK_FALSE;

                        mVulkanCaps.bVK_EXT_buffer_device_address = true;
                        // No need to enable it since it's in vulkan 1.3 core.
                        // Commenting this out fixes an error that both the KHR and EXT versions of this are enabled
                        // extensions.push_back(extension.extensionName);
                    }
                }
            }

            const VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = lastPhysicalDevicePnext,
                .features = {
                    .imageCubeArray = VK_TRUE,
                    .independentBlend = VK_TRUE,
                    .geometryShader = features.geometryShader,
                    .tessellationShader = features.tessellationShader,
                    .sampleRateShading = VK_TRUE,
                    .multiDrawIndirect = VK_TRUE,
                    .drawIndirectFirstInstance = VK_TRUE,
                    .depthClamp = VK_TRUE,
                    .fillModeNonSolid = VK_TRUE,
                    .wideLines = features.wideLines,
                    .samplerAnisotropy = VK_TRUE,
                    .textureCompressionBC = features.textureCompressionBC,
                    .fragmentStoresAndAtomics = VK_TRUE,
                    .shaderImageGatherExtended = VK_TRUE,
                    .shaderStorageImageMultisample = VK_TRUE,
                    .shaderClipDistance = VK_TRUE,
                    .shaderInt64 = mVulkanCaps.bVK_EXT_buffer_device_address ? VK_TRUE : VK_FALSE,
                }
            };


            u32 queueFamilyPropsCount = 0;
            eastl::vector<VkQueueFamilyProperties> queueProps = {};
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyPropsCount, nullptr);
            queueProps.resize(queueFamilyPropsCount);
            vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyPropsCount, queueProps.data());


            eastl::vector<eastl::pair<CommandQueueInfo, u32>> queues = {};

            for (u32 i = 0; i < queueFamilyPropsCount; i++) {
                if (mPresentQueueFamilyIndex == 0xFFFFFFFF &&
                    (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
                    (queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 &&
                    (queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                    mPresentQueueFamilyIndex = i;
                    queues.emplace_back(
                        CommandQueueInfo{ .flags = CommandQueueFlagBits::GRAPHICS | CommandQueueFlagBits::COMPUTE | CommandQueueFlagBits::TRANSFER,
                            .name = "Graphics Command Queue #" + eastl::to_string(i) },
                        i);
                } else {
                    if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT && queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                        Logger::Trace(gVulkanSink, "Found additional graphics queue family {}", i);
                        queues.emplace_back(
                            CommandQueueInfo{ .flags = CommandQueueFlagBits::GRAPHICS | CommandQueueFlagBits::COMPUTE | CommandQueueFlagBits::TRANSFER,
                                .name = "Graphics Command Queue #" + eastl::to_string(i) },
                            i);
                    } else if (queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT && queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                        Logger::Trace(gVulkanSink, "Found additional compute queue family {}", i);
                        queues.emplace_back(
                            CommandQueueInfo{ .flags = CommandQueueFlagBits::COMPUTE | CommandQueueFlagBits::TRANSFER,
                                .name = "Compute Command Queue #" + eastl::to_string(i) },
                            i);
                    } else if (queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                        Logger::Trace(gVulkanSink, "Found additional transfer queue family {}", i);
                        queues.emplace_back(
                            CommandQueueInfo{ .flags = CommandQueueFlagBits::TRANSFER,
                                .name = "Transfer Command Queue #" + eastl::to_string(i) },
                            i);
                    } else {
                        Logger::Warn(gVulkanSink, "Queue family {} has non-standard usages!! Ignoring...", i);
                        continue;
                    }
                }
                mUniqueCommandQueueFamilies.push_back(i);
            }
            if (mPresentQueueFamilyIndex == 0xFFFFFFFF) {
                Logger::Fatal(gVulkanSink, "Failed to find a suitable default command queue!!");
            }

            eastl::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

            for (u32 family : mUniqueCommandQueueFamilies) {
                static const float queuePriority = 0.5f;
                queueCreateInfos.push_back({
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = {},
                    .flags = {},
                    .queueFamilyIndex = family,
                    .queueCount = 1U,
                    .pQueuePriorities = &queuePriority,
                });
            }

            const VkDeviceCreateInfo deviceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = reinterpret_cast<const void*>(&physicalDeviceFeatures2),
                .flags = {},
                .queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = static_cast<u32>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
                .pEnabledFeatures = nullptr,
            };

            auto result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, mContext->GetVkAllocator(), &mDevice);
            CheckVkResult(result);

            volkLoadDevice(mDevice);
            volkLoadDeviceTable(&mDeviceTable, mDevice);

            vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(mContext->GetVkInstance(), "vkSetDebugUtilsObjectNameEXT"));
            vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(mContext->GetVkInstance(), "vkCmdBeginDebugUtilsLabelEXT"));
            vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(mContext->GetVkInstance(), "vkCmdEndDebugUtilsLabelEXT"));
            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                Logger::Debug(gVulkanSink, "vkSetDebugUtilsObjectNameEXT is available");
            }
            if (vkCmdBeginDebugUtilsLabelEXT != nullptr) {
                Logger::Debug(gVulkanSink, "vkCmdBeginDebugUtilsLabelEXT is available");
            }
            if (vkCmdEndDebugUtilsLabelEXT != nullptr) {
                Logger::Debug(gVulkanSink, "vkCmdEndDebugUtilsLabelEXT is available");
            }

            for (const auto& [createInfo, family] : queues) {
                VkQueue queue = VK_NULL_HANDLE;
                vkGetDeviceQueue(mDevice, family, 0, &queue);
                ASSERT(queue != VK_NULL_HANDLE, "Faild to get queue!");
                auto* cqueue = new VulkanCommandQueue(this, queue, family, createInfo);
                if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                    const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                        .pNext = nullptr,
                        .objectType = VK_OBJECT_TYPE_QUEUE,
                        .objectHandle = eastl::bit_cast<uint64_t>(queue),
                        .pObjectName = createInfo.name.c_str(),
                    };
                    vkSetDebugUtilsObjectNameEXT(mDevice, &nameInfo);
                }
                mCommandQueues.push_back(cqueue);
                if (family == mPresentQueueFamilyIndex) {
                    mPresentQueue = cqueue;
                }
            }


            VmaVulkanFunctions vmaVulkanFunctions = {
                .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                .vkAllocateMemory = mDeviceTable.vkAllocateMemory,
                .vkFreeMemory = mDeviceTable.vkFreeMemory,
                .vkMapMemory = mDeviceTable.vkMapMemory,
                .vkUnmapMemory = mDeviceTable.vkUnmapMemory,
                .vkFlushMappedMemoryRanges = mDeviceTable.vkFlushMappedMemoryRanges,
                .vkInvalidateMappedMemoryRanges = mDeviceTable.vkInvalidateMappedMemoryRanges,
                .vkBindBufferMemory = mDeviceTable.vkBindBufferMemory,
                .vkBindImageMemory = mDeviceTable.vkBindImageMemory,
                .vkGetBufferMemoryRequirements = mDeviceTable.vkGetBufferMemoryRequirements,
                .vkGetImageMemoryRequirements = mDeviceTable.vkGetImageMemoryRequirements,
                .vkCreateBuffer = mDeviceTable.vkCreateBuffer,
                .vkDestroyBuffer = mDeviceTable.vkDestroyBuffer,
                .vkCreateImage = mDeviceTable.vkCreateImage,
                .vkDestroyImage = mDeviceTable.vkDestroyImage,
                .vkCmdCopyBuffer = mDeviceTable.vkCmdCopyBuffer,
                .vkGetBufferMemoryRequirements2KHR = mDeviceTable.vkGetBufferMemoryRequirements2KHR,
                .vkGetImageMemoryRequirements2KHR = mDeviceTable.vkGetImageMemoryRequirements2KHR,
                .vkBindBufferMemory2KHR = mDeviceTable.vkBindBufferMemory2KHR,
                .vkBindImageMemory2KHR = mDeviceTable.vkBindImageMemory2KHR,
                .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR,
                .vkGetDeviceBufferMemoryRequirements = mDeviceTable.vkGetDeviceBufferMemoryRequirements,
                .vkGetDeviceImageMemoryRequirements = mDeviceTable.vkGetDeviceImageMemoryRequirements,
            };

            VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
                .physicalDevice = mPhysicalDevice,
                .device = mDevice,
                .preferredLargeHeapBlockSize = 0,
                .pAllocationCallbacks = mContext->GetVkAllocator(),
                .pDeviceMemoryCallbacks = nullptr,
                .pHeapSizeLimit = nullptr,
                .pVulkanFunctions = &vmaVulkanFunctions,
                .instance = mContext->GetVkInstance(),
                .vulkanApiVersion = VK_API_VERSION_1_3,
                .pTypeExternalMemoryHandleTypes = {},
            };

            result = vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator);
            CheckVkResult(result);

            mResourceTable.Initialize(MAX_VK_BINDLESS_BUFFERS, MAX_VK_BINDLESS_IMAGES, MAX_VK_BINDLESS_SAMPLERS,
                16000,
                mDevice, mContext->GetVkAllocator(), VK_NULL_HANDLE, vkSetDebugUtilsObjectNameEXT);

            mMainQueueGpuFence = CreateFence({ .name = "mMainQueueGpuFence" });

            VkPhysicalDeviceProperties physicalDeviceProperties = {};
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

            VkSampleCountFlags colorSampleCounts = physicalDeviceProperties.limits.framebufferColorSampleCounts;
            VkSampleCountFlags depthSampleCounts = physicalDeviceProperties.limits.framebufferDepthSampleCounts;
            VkSampleCountFlags stencilSampleCounts = physicalDeviceProperties.limits.framebufferStencilSampleCounts;
            VkSampleCountFlags counts = colorSampleCounts & depthSampleCounts & stencilSampleCounts;
            VkSampleCountFlags sampledColorSampleCounts = physicalDeviceProperties.limits.sampledImageColorSampleCounts;
            VkSampleCountFlags sampledDepthSampleCounts = physicalDeviceProperties.limits.sampledImageDepthSampleCounts;
            VkSampleCountFlags sampledIntegerSampleCounts = physicalDeviceProperties.limits.sampledImageIntegerSampleCounts;
            VkSampleCountFlags sampledStencilSampleCounts = physicalDeviceProperties.limits.sampledImageStencilSampleCounts;
            VkSampleCountFlags countsSampled = sampledColorSampleCounts & sampledDepthSampleCounts & sampledIntegerSampleCounts & sampledStencilSampleCounts;

            if (counts & VK_SAMPLE_COUNT_64_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e64;
            } else if (counts & VK_SAMPLE_COUNT_32_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e32;
            } else if (counts & VK_SAMPLE_COUNT_16_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e16;
            } else if (counts & VK_SAMPLE_COUNT_8_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e8;
            } else if (counts & VK_SAMPLE_COUNT_4_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e4;
            } else if (counts & VK_SAMPLE_COUNT_2_BIT) {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e2;
            } else {
                mProperties.maxRenderTargetSamples = RasterizationSamples::e1;
            }

            if (countsSampled & VK_SAMPLE_COUNT_64_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e64;
            } else if (countsSampled & VK_SAMPLE_COUNT_32_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e32;
            } else if (countsSampled & VK_SAMPLE_COUNT_16_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e16;
            } else if (countsSampled & VK_SAMPLE_COUNT_8_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e8;
            } else if (countsSampled & VK_SAMPLE_COUNT_4_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e4;
            } else if (countsSampled & VK_SAMPLE_COUNT_2_BIT) {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e2;
            } else {
                mProperties.maxShaderResourceImageSamples = RasterizationSamples::e1;
            }
            // while not required, it's more efficient, so we just enforce this to incur less driver overhead
            mProperties.bufferImageRowAlignment = physicalDeviceProperties.limits.optimalBufferCopyRowPitchAlignment;
            mPhysicalDeviceProperties = physicalDeviceProperties;
        }

        VulkanDevice::~VulkanDevice() {
            WaitIdle();
            CollectGarbage();
            DestroyFence(mMainQueueGpuFence);
            mResourceTable.Cleanup(mDevice, mContext->GetVkAllocator());
            vmaDestroyAllocator(mVmaAllocator);

            for (ICommandQueue* queue : mCommandQueues) {
                delete static_cast<VulkanCommandQueue*>(queue);
            }
            vkDestroyDevice(mDevice, mContext->GetVkAllocator());
        }

        MemoryBlock VulkanDevice::CreateMemoryBlock(const MemoryBlockInfo& info) {
            ASSERT(info.size >= 4);
            ASSERT(PYRO_VERIFY_ALIGNMENT(info.size, 4), "Memory MUST be dword aligned!");
            ASSERT((info.bufferUsage == 0) != (info.imageUsage == 0), "Either buffer usage or image usage must be set!");

            auto [id, ret] = mResourceTable.mVirtualBlockSlots.NewSlot();
            ret.info = info;

            VmaVirtualBlockCreateInfo blockCreateInfo = {};
            blockCreateInfo.size = info.size;
            if (info.strategy == VirtualSuballocationStrategy::AggressiveRing) {
                blockCreateInfo.flags |= VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT;
            }
            blockCreateInfo.pAllocationCallbacks = mContext->GetVkAllocator();
            CheckVkResult(vmaCreateVirtualBlock(&blockCreateInfo, &ret.vmaBlock));
            VmaAllocationInfo vmaAllocationInfo = {};
            VkMemoryPropertyFlags requiredFlags{};
            VkMemoryPropertyFlags preferredFlags{};
            VmaAllocationCreateFlags vmaAllocationFlags{};
            switch (info.domain) {
            case MemoryAllocationDomain::DeviceLocal:
                preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;
            case MemoryAllocationDomain::HostStaging:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case MemoryAllocationDomain::HostRandomWrite:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case MemoryAllocationDomain::HostReadback:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                break;
            }

            if (((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u)) {
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            }

            const VmaAllocationCreateInfo vmaAllocationCreateInfo = {
                .flags = vmaAllocationFlags,
                .usage = VMA_MEMORY_USAGE_UNKNOWN,
                .requiredFlags = requiredFlags,
                .preferredFlags = preferredFlags,
                .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                .pool = VK_NULL_HANDLE,
                .pUserData = nullptr,
                .priority = 0.5f,
            };
            VkDeviceSize requiredAlignment = 1;
            if (info.bufferUsage != 0) {
                if (info.bufferUsage & BufferUsageFlagBits::SHADER_RESOURCE || info.bufferUsage & BufferUsageFlagBits::UNORDERED_ACCESS) {
                    requiredAlignment = eastl::max(requiredAlignment, mPhysicalDeviceProperties.limits.minStorageBufferOffsetAlignment);
                }
                if (info.bufferUsage & BufferUsageFlagBits::UNIFORM_BUFFER) {
                    requiredAlignment = eastl::max(requiredAlignment, mPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
                }
            } else /*imageUsage != 0*/ {
                // At minimum, respect bufferImageGranularity to avoid illegal aliasing.
                requiredAlignment = eastl::max(requiredAlignment, mPhysicalDeviceProperties.limits.bufferImageGranularity);
            }

            ret.requirements.alignment = PYRO_ALIGN(static_cast<VkDeviceSize>(info.minAlignment), requiredAlignment);
            ret.requirements.size = info.size;
            ret.requirements.memoryTypeBits = FindFullMemoryTypeMask(eastl::numeric_limits<u32>::max(), requiredFlags);
            CheckVkResult(vmaAllocateMemory(mVmaAllocator, &ret.requirements, &vmaAllocationCreateInfo, &ret.vmaAllocation, &ret.vmaAllocationInfo));
            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT deviceMemNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_DEVICE_MEMORY,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vmaAllocationInfo.deviceMemory),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &deviceMemNameInfo);
            }
            return eastl::bit_cast<MemoryBlock>(id);
        }

        Buffer VulkanDevice::CreateBuffer(const BufferInfo& info) {
            ASSERT(info.size >= 4);
            auto [id, ret] = mResourceTable.mBufferSlots.NewSlot();
            ret.info = info;

            // NOTE:
            // Zilver - Since vulkan changes certain requirements for memory alignment,
            // I think it still makes sense to add the flags, despite the drivers usually ignoring it for the most part.
            VkBufferUsageFlags VK_BUFFER_USAGE_FLAGS = {};
            if (info.usage & BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS && mVulkanCaps.bVK_EXT_buffer_device_address) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            }
            if (info.usage & BufferUsageFlagBits::TRANSFER_SRC) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            if (info.usage & BufferUsageFlagBits::TRANSFER_DST) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if (info.usage & BufferUsageFlagBits::UNIFORM_BUFFER) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            if (info.usage & BufferUsageFlagBits::UNORDERED_ACCESS || info.usage & BufferUsageFlagBits::SHADER_RESOURCE) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (info.usage & BufferUsageFlagBits::VERTEX_BUFFER) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if (info.usage & BufferUsageFlagBits::INDEX_BUFFER) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if (info.usage & BufferUsageFlagBits::DRAW_INDIRECT) {
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            }
            bool hostAccessible = false;
            VkMemoryPropertyFlags requiredMemoryFlags = {};
            VkMemoryPropertyFlags preferredMemoryFlags = {};
            VmaAllocationInfo vmaAllocationInfo = {};
            VmaAllocationCreateFlags vmaAllocationFlags{};
            VmaMemoryUsage usage = {};
            switch (info.allocationDomain) {
            case MemoryAllocationDomain::DeviceLocal:
                usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                break;
            case MemoryAllocationDomain::HostStaging:
                usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                requiredMemoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case MemoryAllocationDomain::HostRandomWrite:
                usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                requiredMemoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case MemoryAllocationDomain::HostReadback:
                usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                requiredMemoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                preferredMemoryFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                break;
            }

            if (((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u)) {
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                hostAccessible = true;
            }
            const VkBufferCreateInfo vkBufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .size = static_cast<VkDeviceSize>(ret.info.size),
                .usage = VK_BUFFER_USAGE_FLAGS,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = static_cast<u32>(mUniqueCommandQueueFamilies.size()),
                .pQueueFamilyIndices = mUniqueCommandQueueFamilies.data(),
            };
            if (info.memoryBlock == PYRO_NULL_MEMORY_BLOCK) {
                const VmaAllocationCreateInfo vmaAllocationCreateInfo = {
                    .flags = vmaAllocationFlags,
                    .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                    .requiredFlags = requiredMemoryFlags,
                    .preferredFlags = preferredMemoryFlags,
                    .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                    .pool = VK_NULL_HANDLE,
                    .pUserData = nullptr,
                    .priority = 0.5f,
                };
                VkResult result = vmaCreateBuffer(mVmaAllocator, &vkBufferCreateInfo, &vmaAllocationCreateInfo, &ret.vkBuffer, &ret.vmaAllocation.Get<VmaAllocation>(), &vmaAllocationInfo);
                CheckVkResult(result);
                ret.allocationInfo = vmaAllocationInfo;
            } else {
                auto& blockInfo = Slot(info.memoryBlock);
                CheckVkResult(vkCreateBuffer(mDevice, &vkBufferCreateInfo, mContext->GetVkAllocator(), &ret.vkBuffer));
                VkMemoryRequirements requirements;
                vkGetBufferMemoryRequirements(mDevice, ret.vkBuffer, &requirements);
                VmaVirtualAllocationCreateInfo vmaVirtualAllocationCreateInfo = {
                    .size = requirements.size,
                    .alignment = requirements.alignment,
                };
                switch (blockInfo.info.strategy) {
                case VirtualSuballocationStrategy::AggressiveRing:
                case VirtualSuballocationStrategy::TimeEfficient:
                    vmaVirtualAllocationCreateInfo.flags |= VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    vmaVirtualAllocationCreateInfo.flags |= VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
                    break;
                default:
                    break;
                }
                VkDeviceSize offset;
                vmaVirtualAllocate(blockInfo.vmaBlock, &vmaVirtualAllocationCreateInfo, &ret.vmaAllocation.Get<VmaVirtualAllocation>(), &offset);
                CheckVkResult(vkBindBufferMemory(mDevice, ret.vkBuffer, blockInfo.vmaAllocationInfo.deviceMemory, offset));
                vmaGetVirtualAllocationInfo(blockInfo.vmaBlock, ret.vmaAllocation.Get<VmaVirtualAllocation>(), &ret.allocationInfo.Get<VmaVirtualAllocationInfo>());
                ++blockInfo.debugReferences;
            }
            const VkBufferDeviceAddressInfo vkBufferDeviceAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .pNext = nullptr,
                .buffer = ret.vkBuffer,
            };
            if ((info.usage & BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS) && mVulkanCaps.bVK_EXT_buffer_device_address) {
                ret.deviceAddress = vkGetBufferDeviceAddress(mDevice, &vkBufferDeviceAddressInfo);
            }
            ret.hostAddress = hostAccessible ? vmaAllocationInfo.pMappedData : nullptr;
            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT bufferNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_BUFFER,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vkBuffer),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &bufferNameInfo);
            }
            return eastl::bit_cast<Buffer>(id);
        }
        Image VulkanDevice::CreateImage(const ImageInfo& info) {
            auto [id, ret] = mResourceTable.mImageSlots.NewSlot();
            ret.info = info;

            VkImageCreateInfo vkImageCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .format = ToVkFormat(info.format),
                .extent = { info.size.x, info.size.y, info.size.z },
                .mipLevels = info.mipLevelCount,
                .arrayLayers = info.arrayLayerCount,
                .samples = static_cast<VkSampleCountFlagBits>(info.sampleCount),
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = ToVkImageUsageFlags(info.usage, info.format),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = static_cast<u32>(mUniqueCommandQueueFamilies.size()),
                .pQueueFamilyIndices = mUniqueCommandQueueFamilies.data(),
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };

            VmaAllocationCreateFlags vmaAllocationFlags{};
            if (info.arrayLayerCount > 1 && info.dimensions == 2) {
                vkImageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            }
            if (info.flags & ImageCreateFlagBits::CUBE) {
                vkImageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            }
            if (info.flags & ImageCreateFlagBits::ALLOW_ALIAS) {
                vkImageCreateInfo.flags |= VK_IMAGE_CREATE_ALIAS_BIT;
                vmaAllocationFlags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
            }
            if (info.flags & ImageCreateFlagBits::MUTABLE_FORMAT) {
                vkImageCreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            }
            switch (info.dimensions) {
            case 1:
                vkImageCreateInfo.imageType = VK_IMAGE_TYPE_1D;
                break;
            case 2:
                vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                break;
            case 3:
                vkImageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
                break;
            default:
                ASSERT(false, "Invalid image dimensions specified! Valid dimensions are: 1, 2 and 3");
                break;
            }

            if (info.memoryBlock == PYRO_NULL_MEMORY_BLOCK) {
                VmaAllocationInfo vmaAllocationInfo = {};
                const VmaAllocationCreateInfo vmaAllocationCreateInfo = {
                    .flags = vmaAllocationFlags,
                    .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                    .requiredFlags = {},
                    .preferredFlags = {},
                    .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                    .pool = VK_NULL_HANDLE,
                    .pUserData = nullptr,
                    .priority = 0.5f,
                };
                VkResult result = vmaCreateImage(mVmaAllocator, &vkImageCreateInfo, &vmaAllocationCreateInfo, &ret.vkImage, &ret.vmaAllocation.Get<VmaAllocation>(), &vmaAllocationInfo);
                CheckVkResult(result);
                ret.allocationInfo = vmaAllocationInfo;
            } else {
                auto& blockInfo = Slot(info.memoryBlock);
                CheckVkResult(vkCreateImage(mDevice, &vkImageCreateInfo, mContext->GetVkAllocator(), &ret.vkImage));
                VkMemoryRequirements requirements;
                vkGetImageMemoryRequirements(mDevice, ret.vkImage, &requirements);
                VmaVirtualAllocationCreateInfo vmaVirtualAllocationCreateInfo = {
                    .size = requirements.size,
                    .alignment = requirements.alignment,
                };
                switch (blockInfo.info.strategy) {
                case VirtualSuballocationStrategy::TimeEfficient:
                    vmaVirtualAllocationCreateInfo.flags |= VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    vmaVirtualAllocationCreateInfo.flags |= VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
                    break;
                default:
                    break;
                }
                VkDeviceSize offset;
                vmaVirtualAllocate(blockInfo.vmaBlock, &vmaVirtualAllocationCreateInfo, &ret.vmaAllocation.Get<VmaVirtualAllocation>(), &offset);
                CheckVkResult(vkBindImageMemory(mDevice, ret.vkImage, blockInfo.vmaAllocationInfo.deviceMemory, offset));
                vmaGetVirtualAllocationInfo(blockInfo.vmaBlock, ret.vmaAllocation.Get<VmaVirtualAllocation>(), &ret.allocationInfo.Get<VmaVirtualAllocationInfo>());
                ++blockInfo.debugReferences;
            }
            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT imageNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_IMAGE,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vkImage),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &imageNameInfo);
            }

            if (!RHIUtil::FormatIsDepthStencil(info.format)) {
                ret.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            } else {
                if (RHIUtil::FormatHasDepth(info.format))
                    ret.aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
                if (RHIUtil::FormatHasStencil(info.format))
                    ret.aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            return eastl::bit_cast<Image>(id);
        }

        ShaderResourceId VulkanDevice::CreateShaderResource(const GPUResourceInfo& info) {
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                return ShaderResourceId{ CreateBufferView(info) };
            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                return ShaderResourceId{ CreateImageView(info, false) };
            } else {
                ASSERT(false, "Bad variant");
            }
            return ShaderResourceId{};
        }

        UnorderedAccessId VulkanDevice::CreateUnorderedAccess(const GPUResourceInfo& info) {
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                return UnorderedAccessId{ CreateBufferView(info) };
            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                return UnorderedAccessId{ CreateImageView(info, true) };
            } else {
                ASSERT(false, "Bad variant");
            }
            return UnorderedAccessId{};
        }

        GPUResourceId VulkanDevice::CreateBufferView(const GPUResourceInfo& info) {
            auto [id, ret] = mResourceTable.mResourceViewSlots.NewSlot();
            ret.info = info;
            auto& resourceInfo = eastl::get<BufferResourceInfo>(info);
            ImplBufferSlot& bufferSlot = Slot(resourceInfo.buffer);
            ret.descriptor.Get<VkDescriptorBufferInfo>() = {
                .buffer = bufferSlot.vkBuffer,
                .offset = resourceInfo.region.offset,
                .range = resourceInfo.region.size,
            };

            mResourceTable.WriteDescriptorSetBuffer(mDevice, mResourceTable.mBindlessDescriptorSet,
                ret.descriptor.Get<VkDescriptorBufferInfo>().buffer, resourceInfo.region.offset, resourceInfo.region.size, id.index);

            return id;
        }

        GPUResourceId VulkanDevice::CreateImageView(const GPUResourceInfo& info, bool uav) {
            // ASSERT(!(info.mipmapFilter != Filter::CubicImg));

            auto [id, ret] = mResourceTable.mResourceViewSlots.NewSlot();
            ret.info = info;
            auto& resourceInfo = eastl::get<ImageResourceInfo>(info);

            ImplImageSlot& imageSlot = Slot(resourceInfo.image);

            const VkImageViewCreateInfo vkImageViewCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = imageSlot.vkImage,
                .viewType = ToVkImageViewType(resourceInfo.viewType),
                .format = ToVkFormat(resourceInfo.format == Format::Inherit ? imageSlot.info.format : resourceInfo.format),
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = imageSlot.aspectFlags,
                    .baseMipLevel = resourceInfo.slice.baseMipLevel,
                    .levelCount = resourceInfo.slice.levelCount,
                    .baseArrayLayer = resourceInfo.slice.baseArrayLayer,
                    .layerCount = resourceInfo.slice.layerCount,
                },
            };

            VkResult result = vkCreateImageView(mDevice, &vkImageViewCreateInfo, mContext->GetVkAllocator(),
                &ret.descriptor.Get<VkDescriptorImageInfo>().imageView);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT) {
                eastl::string name =
                    imageSlot.info.name + " (View: levels [" + eastl::to_string(resourceInfo.slice.baseMipLevel) +
                    ", " + eastl::to_string(resourceInfo.slice.baseMipLevel + resourceInfo.slice.levelCount) + "], layers [" + eastl::to_string(resourceInfo.slice.baseArrayLayer) +
                    ", " + eastl::to_string(resourceInfo.slice.baseMipLevel + resourceInfo.slice.levelCount) + "])";

                const VkDebugUtilsObjectNameInfoEXT imageViewNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.descriptor.Get<VkDescriptorImageInfo>().imageView),
                    .pObjectName = name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &imageViewNameInfo);
            }
            if (uav) {
                ret.descriptor.Get<VkDescriptorImageInfo>().imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            } else {
                ret.descriptor.Get<VkDescriptorImageInfo>().imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            }

            mResourceTable.WriteDescriptorSetImageView(mDevice, mResourceTable.mBindlessDescriptorSet,
                ret.descriptor.Get<VkDescriptorImageInfo>().imageView, imageSlot.info.usage, id.index);

            return id;
        }

        SamplerId VulkanDevice::CreateSampler(const SamplerInfo& info) {
            auto [id, ret] = mResourceTable.mSamplerSlots.NewSlot();
            ret.info = info;

            VkSamplerReductionModeCreateInfo vkSamplerReductionModeCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
                .pNext = nullptr,
                .reductionMode = ToVkReductionMode(ret.info.reductionMode),
            };

            const VkSamplerCreateInfo vkSamplerCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = reinterpret_cast<void*>(&vkSamplerReductionModeCreateInfo),
                .flags = {},
                .magFilter = ToVkFilter(ret.info.magnificationFilter),
                .minFilter = ToVkFilter(ret.info.minificationFilter),
                .mipmapMode = ret.info.mipmapFilter == RHI::Filter::Nearest ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = ToVkSamplerAddressMode(ret.info.addressModeU),
                .addressModeV = ToVkSamplerAddressMode(ret.info.addressModeV),
                .addressModeW = ToVkSamplerAddressMode(ret.info.addressModeW),
                .mipLodBias = ret.info.mipLodBias,
                .anisotropyEnable = static_cast<VkBool32>(ret.info.enableAnisotropy),
                .maxAnisotropy = static_cast<float>(ret.info.maxAnisotropy),
                .compareEnable = static_cast<VkBool32>(ret.info.enableCompare),
                .compareOp = ToVkCompareOp(ret.info.compareOp),
                .minLod = ret.info.minLod,
                .maxLod = eastl::clamp(ret.info.maxLod, 0.0f, VK_LOD_CLAMP_NONE),
                .borderColor = ToVkBorderColor(info.borderColor),
                .unnormalizedCoordinates = VK_FALSE
            };

            VkResult result = vkCreateSampler(mDevice, &vkSamplerCreateInfo, mContext->GetVkAllocator(), &ret.vkSampler);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT) {

                const VkDebugUtilsObjectNameInfoEXT samplerNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SAMPLER,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vkSampler),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &samplerNameInfo);
            }

            mResourceTable.WriteDescriptorSetSampler(mDevice, mResourceTable.mBindlessDescriptorSet, ret.vkSampler, id.index);

            return SamplerId{ id };
        }

        RenderTarget VulkanDevice::CreateRenderTarget(const RenderTargetInfo& info) {
            return RenderTarget(new VulkanRenderTarget(this, info));
        }

        const MemoryBlockInfo& VulkanDevice::GetMemoryBlockInfo(MemoryBlock memory) const {
            return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(memory)).info;
        }

        const BufferInfo& VulkanDevice::GetBufferInfo(Buffer buffer) const {
            return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(buffer)).info;
        }

        const ImageInfo& VulkanDevice::GetImageInfo(Image image) const {
            return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(image)).info;
        }

        const GPUResourceInfo& VulkanDevice::GetShaderResourceInfo(ShaderResourceId id) const {
            return mResourceTable.mResourceViewSlots.DereferenceId(id).info;
        }
        const GPUResourceInfo& VulkanDevice::GetUnorderedAccessInfo(UnorderedAccessId id) const {
            return mResourceTable.mResourceViewSlots.DereferenceId(id).info;
        }

        const SamplerInfo& VulkanDevice::GetSamplerInfo(SamplerId id) const {
            return mResourceTable.mSamplerSlots.DereferenceId(id).info;
        }

        const RenderTargetInfo& VulkanDevice::GetRenderTargetInfo(RenderTarget renderTarget) const {
            return eastl::bit_cast<VulkanRenderTarget*>(renderTarget)->Info();
        }

        const RasterPipelineInfo& VulkanDevice::GetRasterPipelineInfo(RasterPipeline pipeline) const {
            return eastl::bit_cast<VulkanRasterPipeline*>(pipeline)->Info();
        }

        const ComputePipelineInfo& VulkanDevice::GetComputePipelineInfo(ComputePipeline pipeline) const {
            return eastl::bit_cast<VulkanComputePipeline*>(pipeline)->Info();
        }

        const SemaphoreInfo& VulkanDevice::GetSemaphoreInfo(Semaphore semaphore) const {
            return eastl::bit_cast<VulkanSemaphore*>(semaphore)->Info();
        }

        DeviceAddress VulkanDevice::BufferDeviceAddress(Buffer buffer) const {
            return DeviceAddress();
        }

        u8* VulkanDevice::BufferHostAddress(Buffer buffer) const {
            return reinterpret_cast<u8*>(Slot(buffer).hostAddress);
        }

        DeviceSize VulkanDevice::ImageSizeRequirements(Image image) const {
            auto& img = Slot(image);
            if (img.info.memoryBlock) {
                return img.allocationInfo.Get<VmaVirtualAllocationInfo>().size;
            } else {
                return img.allocationInfo.Get<VmaAllocationInfo>().size;
            }
        }
        u32 VulkanDevice::ImageSubresourceRowPitch(Image image, ImageSlice slice, u32 rowWidth) const {
            return PYRO_ALIGN(rowWidth, mPhysicalDeviceProperties.limits.optimalBufferCopyRowPitchAlignment);
        }

        bool VulkanDevice::IsMemoryBlockValid(MemoryBlock memory) const {
            return mResourceTable.mVirtualBlockSlots.IsIdValid(eastl::bit_cast<GPUResourceId>(memory));
        }

        bool VulkanDevice::IsBufferValid(Buffer buffer) const {
            return mResourceTable.mBufferSlots.IsIdValid(eastl::bit_cast<GPUResourceId>(buffer));
        }

        bool VulkanDevice::IsImageValid(Image image) const {
            return mResourceTable.mImageSlots.IsIdValid(eastl::bit_cast<GPUResourceId>(image));
        }

        bool VulkanDevice::IsShaderResourceValid(ShaderResourceId id) const {
            return mResourceTable.mResourceViewSlots.IsIdValid(id);
        }
        bool VulkanDevice::IsUnorderedAccessValid(UnorderedAccessId id) const {
            return mResourceTable.mResourceViewSlots.IsIdValid(id);
        }

        bool VulkanDevice::IsSamplerValid(SamplerId id) const {
            return mResourceTable.mSamplerSlots.IsIdValid(id);
        }

        void VulkanDevice::DestroyRenderTarget(RenderTarget& renderTarget) {
            VulkanRenderTarget* targ = eastl::bit_cast<VulkanRenderTarget*>(renderTarget);
            delete targ;
            renderTarget = nullptr;
        }
        void VulkanDevice::DestroyRasterPipeline(RasterPipeline& pipeline) {
            delete eastl::bit_cast<VulkanRasterPipeline*>(pipeline);
            pipeline = nullptr;
        }
        void VulkanDevice::DestroyComputePipeline(ComputePipeline& pipeline) {
            delete eastl::bit_cast<VulkanComputePipeline*>(pipeline);
            pipeline = nullptr;
        }
        void VulkanDevice::DestroySwapChain(ISwapChain*& swapChain) {
            ASSERT(dynamic_cast<VulkanSwapChain*>(swapChain) != nullptr, "Must be of type VulkanSwapChain!");
            delete static_cast<VulkanSwapChain*>(swapChain);
            swapChain = nullptr;
        }

        void VulkanDevice::DestroyMemoryBlock(MemoryBlock& memory) {
            ImplVmaVirtualBlockSlot& blockSlot = Slot(memory);
            ASSERT(blockSlot.debugReferences == 0, "Not all references to this MemoryBlock have been freed yet!");
            vmaFreeMemory(mVmaAllocator, blockSlot.vmaAllocation);
            vmaDestroyVirtualBlock(blockSlot.vmaBlock);
            blockSlot = {};
            mResourceTable.mVirtualBlockSlots.ReturnSlot(eastl::bit_cast<GPUResourceId>(memory));
            memory = PYRO_NULL_MEMORY_BLOCK;
        }

        void VulkanDevice::DestroyBuffer(Buffer& buffer) {
            ImplBufferSlot& bufferSlot = Slot(buffer);
            if (bufferSlot.info.memoryBlock) {
                auto& block = Slot(bufferSlot.info.memoryBlock);
                --block.debugReferences;
                vkDestroyBuffer(mDevice, bufferSlot.vkBuffer, mContext->GetVkAllocator());
                vmaVirtualFree(block.vmaBlock, bufferSlot.vmaAllocation.Get<VmaVirtualAllocation>());
            } else {
                vmaDestroyBuffer(mVmaAllocator, bufferSlot.vkBuffer, bufferSlot.vmaAllocation.Get<VmaAllocation>());
            }
            bufferSlot = {};
            mResourceTable.mBufferSlots.ReturnSlot(eastl::bit_cast<GPUResourceId>(buffer));
            buffer = PYRO_NULL_BUFFER;
        }

        void VulkanDevice::DestroyImage(Image& image) {
            ImplImageSlot& imageSlot = Slot(image);
            if (imageSlot.swapchainImageIndex == NOT_OWNED_BY_SWAPCHAIN) {
                if (imageSlot.info.memoryBlock) {
                    auto& block = Slot(imageSlot.info.memoryBlock);
                    --block.debugReferences;
                    vkDestroyImage(mDevice, imageSlot.vkImage, mContext->GetVkAllocator());
                    vmaVirtualFree(block.vmaBlock, imageSlot.vmaAllocation.Get<VmaVirtualAllocation>());
                } else {
                    vmaDestroyImage(mVmaAllocator, imageSlot.vkImage, imageSlot.vmaAllocation.Get<VmaAllocation>());
                }
            }
            imageSlot = {};
            mResourceTable.mImageSlots.ReturnSlot(eastl::bit_cast<GPUResourceId>(image));
            image = PYRO_NULL_IMAGE;
        }

        void VulkanDevice::DestroyShaderResource(ShaderResourceId& srv) {
            ImplResourceViewSlot& srvSlot = Slot(srv);
            if (eastl::holds_alternative<ImageResourceInfo>(srvSlot.info)) {
                vkDestroyImageView(mDevice, srvSlot.descriptor.Get<VkDescriptorImageInfo>().imageView, mContext->GetVkAllocator());
            }
            srvSlot = {};
            mResourceTable.mResourceViewSlots.ReturnSlot(srv);
            srv = PYRO_NULL_SRV;
        }
        void VulkanDevice::DestroyUnorderedAccess(UnorderedAccessId& uav) {
            ImplResourceViewSlot& uavSlot = Slot(uav);
            if (eastl::holds_alternative<ImageResourceInfo>(uavSlot.info)) {
                vkDestroyImageView(mDevice, uavSlot.descriptor.Get<VkDescriptorImageInfo>().imageView, mContext->GetVkAllocator());
            }
            uavSlot = {};
            mResourceTable.mResourceViewSlots.ReturnSlot(uav);
            uav = PYRO_NULL_UAV;
        }

        void VulkanDevice::DestroySampler(SamplerId& sampler) {
            ImplSamplerSlot& samplerSlot = mResourceTable.mSamplerSlots.DereferenceId(sampler);
            vkDestroySampler(mDevice, samplerSlot.vkSampler, mContext->GetVkAllocator());
            samplerSlot = {};
            mResourceTable.mSamplerSlots.ReturnSlot(sampler);
            sampler = PYRO_NULL_SAMPLER;
        }

        void VulkanDevice::DestroySemaphore(Semaphore& semaphore) {
            VulkanSemaphore* sem = eastl::bit_cast<VulkanSemaphore*>(semaphore);
            delete sem;
            semaphore = nullptr;
        }

        void VulkanDevice::DestroyFence(IFence*& fence) {
            ASSERT(dynamic_cast<VulkanFence*>(fence) != nullptr, "Must be of type VulkanFence!");
            VulkanFence* fen = static_cast<VulkanFence*>(fence);
            delete fen;
            fence = nullptr;
        }

        void VulkanDevice::DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool) {
            VulkanTimestampQueryPool* q = static_cast<VulkanTimestampQueryPool*>(queryPool);
            delete q;
            queryPool = nullptr;
        }

        VulkanSwapChainSupportInfo VulkanDevice::GetSwapChainSupport(VkSurfaceKHR surface) const {
            VulkanSwapChainSupportInfo support{};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, surface, &support.capabilities);
            u32 formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                support.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &formatCount, support.formats.data());
            }

            u32 presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                support.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, support.presentModes.data());
            }
            return support;
        }

        uint32_t VulkanDevice::FindFullMemoryTypeMask(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties) {
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProps);
            uint32_t mask = 0;
            for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
                if ((memoryTypeBits & (1 << i)) &&
                    (memProps.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties) {
                    mask |= 1 << i;
                }
            }
            if (mask == 0) {
                Logger::Fatal(gVulkanSink, "failed to find a suitable memory type!");
            }
            return mask;
        }

        RasterPipeline VulkanDevice::CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) {
            return RasterPipeline(new VulkanRasterPipeline(this, info, rasterShaderStages));
        }

        ComputePipeline VulkanDevice::CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) {
            return ComputePipeline(new VulkanComputePipeline(this, info, computeShaderInfo));
        }


        ISwapChain* VulkanDevice::CreateSwapChain(const SwapChainInfo& info) {
            VulkanSwapChain* swapchain = new VulkanSwapChain(this, info);
            mActiveSwapChains.emplace(swapchain);
            return swapchain;
        }

        Semaphore VulkanDevice::CreateSemaphore(const SemaphoreInfo& info) {
            VkSemaphoreCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
            };

            VkSemaphore semaphore;
            VkResult result = vkCreateSemaphore(mDevice, &createInfo, mContext->GetVkAllocator(), &semaphore);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT samplerNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                    .objectHandle = eastl::bit_cast<uint64_t>(semaphore),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &samplerNameInfo);
            }
            return Semaphore(new VulkanSemaphore(semaphore, this, info));
        }

        IFence* VulkanDevice::CreateFence(const FenceInfo& info) {
            VkSemaphoreTypeCreateInfo semaphoreType{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = nullptr,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue = info.initialValue,
            };

            VkSemaphoreCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = &semaphoreType,
                .flags = {},
            };

            VkSemaphore semaphore;
            VkResult result = vkCreateSemaphore(mDevice, &createInfo, mContext->GetVkAllocator(), &semaphore);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT samplerNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                    .objectHandle = eastl::bit_cast<uint64_t>(semaphore),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &samplerNameInfo);
            }
            return new VulkanFence(semaphore, this, info);
        }

        ITimestampQueryPool* VulkanDevice::CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) {
            return new VulkanTimestampQueryPool(this, info);
        }

        eastl::optional<Format> VulkanDevice::PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) {
            VkFormatFeatureFlags flags = ToVkFormatFeatureFlags(features);
            for (Format format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, ToVkFormat(format), &props);
                if ((props.optimalTilingFeatures & flags) == flags) {
                    return format;
                }
            }
            return eastl::optional<Format>{};
        }

        void VulkanDevice::WaitIdle() {
            for (ICommandQueue* queue : mCommandQueues) {
                queue->WaitIdle();
            }
            vkDeviceWaitIdle(mDevice);
        }

        void VulkanDevice::SubmitQueue(const CommandQueueSubmitInfo& info) {
            const u64 currentMainQueueCpuTimelineValue = mMainQueueCpuTimeline.fetch_add(1) + 1;
            CollectGarbage();

            VulkanCommandQueue* vkQueue = static_cast<VulkanCommandQueue*>(info.queue);
            ASSERT(!vkQueue->mbPendingSwapPresent, "Queue must have been presented!");
            if (vkQueue->RefSubmittedSwapChains().size() > 0) {
                vkQueue->mbPendingSwapPresent = true;
            }
            VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };

            eastl::vector<VkCommandBufferSubmitInfo> vkCommandBuffers = {};
            for (VulkanCommandBuffer* commandBuffer : vkQueue->RefSubmittedCommandBuffers()) {
                vkCommandBuffers.push_back({ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .commandBuffer = commandBuffer->GetVkCommandBuffer() });
            }
            submitInfo.pCommandBufferInfos = vkCommandBuffers.data();
            submitInfo.commandBufferInfoCount = vkCommandBuffers.size();

            eastl::vector<VkSemaphoreSubmitInfo> waitSemaphores = {};
            eastl::vector<VkSemaphoreSubmitInfo> signalSemaphores = {};
            {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = static_cast<VulkanFence*>(mMainQueueGpuFence)->GetVkSemaphore();
                semaphoreSubmit.value = currentMainQueueCpuTimelineValue;
                semaphoreSubmit.stageMask = /*FIXME: is this efficient? -> */ VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                signalSemaphores.push_back(semaphoreSubmit);
            }
            for (auto [fence, index] : info.signalFences) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = static_cast<VulkanFence*>(fence)->GetVkSemaphore();
                semaphoreSubmit.value = index;
                semaphoreSubmit.stageMask = /*FIXME: is this efficient? -> */ VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                signalSemaphores.push_back(semaphoreSubmit);
            }
            for (auto [semaphore, stage] : info.signalPresentReadySemaphores) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = ToVkPipelineStageFlags(stage);
                signalSemaphores.push_back(semaphoreSubmit);
            }
            for (auto [semaphore, stage] : info.signalSemaphores) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = ToVkPipelineStageFlags(stage);
                signalSemaphores.push_back(semaphoreSubmit);
            }

            eastl::vector<VkPipelineStageFlags2> waitStageMasks = {};
            waitStageMasks.reserve(info.waitSemaphores.size() + vkQueue->RefSubmittedSwapAcquireSemaphores().size());

            for (auto [semaphore, stage] : info.waitSemaphores) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = ToVkPipelineStageFlags(stage);
                waitSemaphores.push_back(semaphoreSubmit);
            }
            // TODO for Lukas, add timeline semaphores here?
            for (auto semaphore : vkQueue->RefSubmittedSwapAcquireSemaphores()) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = /*FIXME: is this efficient? -> */ VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                waitSemaphores.push_back(semaphoreSubmit);
            }

            submitInfo.waitSemaphoreInfoCount = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
            submitInfo.signalSemaphoreInfoCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();

            vkQueueSubmit2(vkQueue->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);

            for (VulkanCommandBuffer* commandBuffer : vkQueue->RefSubmittedCommandBuffers()) {
                delete commandBuffer;
            }

            vkQueue->RefSubmittedCommandBuffers().clear();
        }

        void VulkanDevice::PresentQueue(const CommandQueuePresentInfo& info) {
            VulkanCommandQueue* vkQueue = static_cast<VulkanCommandQueue*>(info.queue);
            vkQueue->mbPendingSwapPresent = false;
            eastl::vector<VkResult> result(vkQueue->RefSubmittedSwapChains().size());

            eastl::vector<VkSemaphore> waitSemaphores = {};
            for (auto semaphore : info.waitSemaphores) {
                waitSemaphores.push_back(eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore());
            }

            if (!vkQueue->RefSubmittedSwapChains().empty()) {
                VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
                presentInfo.pImageIndices = vkQueue->RefSubmittedSwapChainImageIndices().data();
                presentInfo.pSwapchains = vkQueue->RefSubmittedSwapChains().data();
                presentInfo.swapchainCount = static_cast<u32>(vkQueue->RefSubmittedSwapChains().size());
                presentInfo.pWaitSemaphores = waitSemaphores.data();
                presentInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
                presentInfo.pResults = result.data();
                vkQueuePresentKHR(vkQueue->GetVkQueue(), &presentInfo);
                vkQueue->RefSubmittedSwapChains().clear();
                vkQueue->RefSubmittedSwapChainImageIndices().clear();
                vkQueue->RefSubmittedSwapAcquireSemaphores().clear();
            }
        }

        void VulkanDevice::CollectGarbage() {
            const u64 currentMainQueueGpuTimelineValue = mMainQueueGpuFence->Value();

            auto CheckAndCleanupGpuResources = [&](auto& zombies, const auto& cleanupFn) {
                while (!zombies.empty()) {
                    auto& [timelineValue, object] = zombies.back();

                    if (timelineValue > currentMainQueueGpuTimelineValue) {
                        break;
                    }

                    cleanupFn(object);
                    zombies.pop_back();
                }
            };

            CheckAndCleanupGpuResources(mMainQueueCommandListZombies, [&](auto& zombie) {
                for (auto resource : zombie.zombies->zombies) {
                    mMainQueueZombies.emplace_front(mMainQueueCpuTimeline, resource);
                }
                vkResetCommandPool(mDevice, zombie.vkCmdPool, {});
                zombie.queue->GetCommandBufferPool()->PutBack({ zombie.vkCmdPool, zombie.vkCmdBuffer });
            });

            CheckAndCleanupGpuResources(mMainQueueZombies, [this](ZombieDeleter& zombie) { zombie.deleter(this, zombie.resource); });
        }

        const DeviceInfo& VulkanDevice::GetInfo() {
            ASSERT(false, "TODO");
            return mInfo;
        }
        const DevicePropertiesInfo& VulkanDevice::GetProperties() {
            return mProperties;
        }

        Image VulkanDevice::NewSwapChainImage(VkImage swapchainImage, VkFormat format, u32 index, ImageUsageFlags usage, const ImageInfo& imageInfo) {
            auto [id, image_slot] = mResourceTable.mImageSlots.NewSlot();

            ImplImageSlot ret;
            ret.vkImage = swapchainImage;
            if (!RHIUtil::FormatIsDepthStencil(imageInfo.format)) {
                ret.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            } else {
                if (RHIUtil::FormatHasDepth(imageInfo.format))
                    ret.aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
                if (RHIUtil::FormatHasStencil(imageInfo.format))
                    ret.aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            VkImageViewCreateInfo const viewCi{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = swapchainImage,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            ret.swapchainImageIndex = static_cast<i32>(index);
            ret.info = imageInfo;

            if (vkSetDebugUtilsObjectNameEXT && !imageInfo.name.empty()) {
                auto swapchain_image_name = imageInfo.name;
                VkDebugUtilsObjectNameInfoEXT const swapchain_image_name_info{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_IMAGE,
                    .objectHandle = reinterpret_cast<uint64_t>(ret.vkImage),
                    .pObjectName = swapchain_image_name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(this->mDevice, &swapchain_image_name_info);
            }

            image_slot = ret;

            return eastl::bit_cast<Image>(id);
        }
        auto VulkanDevice::Slot(MemoryBlock id) -> ImplVmaVirtualBlockSlot& { return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(Buffer id) -> ImplBufferSlot& { return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(Image id) -> ImplImageSlot& { return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(ShaderResourceId id) -> ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(UnorderedAccessId id) -> ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(SamplerId id) -> ImplSamplerSlot& { return mResourceTable.mSamplerSlots.DereferenceId(id); }

        auto VulkanDevice::Slot(MemoryBlock id) const -> const ImplVmaVirtualBlockSlot& { return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(Buffer id) const -> const ImplBufferSlot& { return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(Image id) const -> const ImplImageSlot& { return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GPUResourceId>(id)); }
        auto VulkanDevice::Slot(ShaderResourceId id) const -> const ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(UnorderedAccessId id) const -> const ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(SamplerId id) const -> const ImplSamplerSlot& { return mResourceTable.mSamplerSlots.DereferenceId(id); }
    } // namespace RHIVulkan
} // namespace PyroshockStudios