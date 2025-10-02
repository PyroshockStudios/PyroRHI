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
                        extensions.push_back(extension.extensionName);
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

            CommandQueueInfo queueCreateInfo{};
            queueCreateInfo.name = "General Purpose Queue";
            queueCreateInfo.bPresentable = true;
            u32 queueFamilyIndex = 0xFFFFFFFF;
            for (u32 i = 0; i < queueFamilyPropsCount; i++) {
                if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
                    (queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 &&
                    (queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                    queueFamilyIndex = i;
                    queueCreateInfo.flags = CommandQueueFlagBits::COMPUTE | CommandQueueFlagBits::GRAPHICS | CommandQueueFlagBits::TRANSFER;
                    mUniqueCommandQueueFamilies.push_back(i);
                    break;
                }
            }
            if (queueFamilyIndex == 0xFFFFFFFF) {
                Logger::Fatal(gVulkanSink, "Failed to find a suitable command queue!!");
            }

            eastl::array<f32, 1> queue_priorities = { queueCreateInfo.priority };
            const VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = {},
                .flags = {},
                .queueFamilyIndex = queueFamilyIndex,
                .queueCount = static_cast<u32>(queue_priorities.size()),
                .pQueuePriorities = queue_priorities.data(),
            };

            const VkDeviceCreateInfo deviceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = reinterpret_cast<const void*>(&physicalDeviceFeatures2),
                .flags = {},
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &vkDeviceQueueCreateInfo,
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
                Logger::Info(gVulkanSink, "vkSetDebugUtilsObjectNameEXT is available");
            }
            if (vkCmdBeginDebugUtilsLabelEXT != nullptr) {
                Logger::Info(gVulkanSink, "vkCmdBeginDebugUtilsLabelEXT is available");
            }
            if (vkCmdEndDebugUtilsLabelEXT != nullptr) {
                Logger::Info(gVulkanSink, "vkCmdEndDebugUtilsLabelEXT is available");
            }
            VkQueue queue = VK_NULL_HANDLE;
            vkGetDeviceQueue(mDevice, queueFamilyIndex, 0, &queue);
            mPresentQueue = new VulkanCommandQueue(this, queue, queueFamilyIndex, queueCreateInfo);
            if (vkSetDebugUtilsObjectNameEXT != nullptr) {
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_QUEUE,
                    .objectHandle = eastl::bit_cast<uint64_t>(queue),
                    .pObjectName = queueCreateInfo.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &nameInfo);
            }
            mCommandQueues.push_back(mPresentQueue);

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
                mDevice, mContext->GetVkAllocator(), VK_NULL_HANDLE, vkSetDebugUtilsObjectNameEXT);

            mCommandBufferPool = new CommandBufferPool();

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

            mCommandBufferPool->Cleanup(this);
            delete mCommandBufferPool;

            vmaDestroyAllocator(mVmaAllocator);

            for (ICommandQueue* queue : mCommandQueues) {
                delete static_cast<VulkanCommandQueue*>(queue);
            }
            vkDestroyDevice(mDevice, mContext->GetVkAllocator());
        }

        MemoryBlock VulkanDevice::CreateMemoryBlock(const MemoryBlockInfo& info) {
            ASSERT(info.size >= 4);
            ASSERT(PYRO_VERIFY_ALIGNMENT(info.size, 4), "Memory MUST be dword aligned!");

            auto [id, ret] = mResourceTable.mVirtualBlockSlots.NewSlot();
            ret.info = info;

            VmaVirtualBlockCreateInfo blockCreateInfo = {};
            blockCreateInfo.size = info.size;
            if (info.strategy == VirtualSuballocationStrategy::Fast) {
                blockCreateInfo.flags |= VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT;
            }
            blockCreateInfo.pAllocationCallbacks = mContext->GetVkAllocator();
            CheckVkResult(vmaCreateVirtualBlock(&blockCreateInfo, &ret.vmaBlock));

            VmaAllocationInfo vmaAllocationInfo = {};
            VmaAllocationCreateFlags vmaAllocationFlags{};
            switch (info.domain) {
            case MemoryAllocationDomain::DeviceLocal:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                break;
            case MemoryAllocationDomain::HostStaging:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                break;
            case MemoryAllocationDomain::HostRandomWrite:
            case MemoryAllocationDomain::HostReadback:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                break;
            }

            if (((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) != 0u) ||
                ((vmaAllocationFlags & VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) != 0u)) {
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                hostAccessible = true;
            }
        
            const VmaAllocationCreateInfo vmaAllocationCreateInfo = {
                .flags = vmaAllocationFlags,
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags = {},
                .preferredFlags = {},
                .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                .pool = nullptr,
                .pUserData = nullptr,
                .priority = 0.5f,
            };


            return eastl::bit_cast<MemoryBlock>(id);
        }

        Buffer VulkanDevice::CreateBuffer(const BufferInfo& info) {
            ASSERT(info.size >= 4);
            auto [id, ret] = mResourceTable.mBufferSlots.NewSlot();
            ret.info = info;
            static constexpr VkBufferUsageFlags VK_BUFFER_USAGE_FLAGS =
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            bool hostAccessible = false;
            VmaAllocationInfo vmaAllocationInfo = {};
            VmaAllocationCreateFlags vmaAllocationFlags{};
            switch (info.allocationDomain) {
            case MemoryAllocationDomain::DeviceLocal:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                break;
            case MemoryAllocationDomain::HostStaging:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                break;
            case MemoryAllocationDomain::HostRandomWrite:
            case MemoryAllocationDomain::HostReadback:
                vmaAllocationFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
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
                    .requiredFlags = {},
                    .preferredFlags = {},
                    .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                    .pool = nullptr,
                    .pUserData = nullptr,
                    .priority = 0.5f,
                };
                VkResult result = vmaCreateBuffer(mVmaAllocator, &vkBufferCreateInfo, &vmaAllocationCreateInfo, &ret.vkBuffer, &ret.vmaAllocation, &vmaAllocationInfo);
                CheckVkResult(result);
                ret.allocationInfo = vmaAllocationInfo;
            } else {
                 VmaVirtualAllocationCreateInfo vmaVirtualAllocationCreateInfo = {
                    .size = 
                    .alignment 
                }
            }
            const VkBufferDeviceAddressInfo vkBufferDeviceAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .pNext = nullptr,
                .buffer = ret.vkBuffer,
            };
            ret.deviceAddress = vkGetBufferDeviceAddress(mDevice, &vkBufferDeviceAddressInfo);
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

            VmaAllocationInfo vmaAllocationInfo = {};
            const VmaAllocationCreateInfo vmaAllocationCreateInfo = {
                .flags = vmaAllocationFlags,
                .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags = {},
                .preferredFlags = {},
                .memoryTypeBits = eastl::numeric_limits<u32>::max(),
                .pool = nullptr,
                .pUserData = nullptr,
                .priority = 0.5f,
            };
            VkResult result = vmaCreateImage(mVmaAllocator, &vkImageCreateInfo, &vmaAllocationCreateInfo, &ret.vkImage, &ret.vmaAllocation, &vmaAllocationInfo);
            CheckVkResult(result);
            ret.allocationInfo = vmaAllocationInfo;
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
            // ASSERT(!(info.mipmapFilter != Filter::CubicImg));

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
            return Slot(image).allocationInfo.size;
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
            vmaDestroyVirtualBlock(blockSlot.vmaBlock);
            blockSlot = {};
            mResourceTable.mVirtualBlockSlots.ReturnSlot(eastl::bit_cast<GPUResourceId>(memory));
            memory = PYRO_NULL_MEMORY_BLOCK;
        }

        void VulkanDevice::DestroyBuffer(Buffer& buffer) {
            ImplBufferSlot& bufferSlot = Slot(buffer);
            vmaDestroyBuffer(mVmaAllocator, bufferSlot.vkBuffer, bufferSlot.vmaAllocation);
            bufferSlot = {};
            mResourceTable.mBufferSlots.ReturnSlot(eastl::bit_cast<GPUResourceId>(buffer));
            buffer = PYRO_NULL_BUFFER;
        }

        void VulkanDevice::DestroyImage(Image& image) {
            ImplImageSlot& imageSlot = Slot(image);
            if (imageSlot.swapchainImageIndex == NOT_OWNED_BY_SWAPCHAIN) {
                vmaDestroyImage(mVmaAllocator, imageSlot.vkImage, imageSlot.vmaAllocation);
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

        ICommandBuffer* VulkanDevice::GetCommandBuffer(const CommandBufferInfo& info) {
            auto [pool, buffer] = mCommandBufferPool->Get(this);
            return new VulkanCommandBuffer(this, pool, buffer, info);
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
            VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

            eastl::vector<VkCommandBuffer> vkCommandBuffers = {};
            for (VulkanCommandBuffer* commandBuffer : vkQueue->RefSubmittedCommandBuffers()) {
                vkCommandBuffers.push_back(commandBuffer->GetVkCommandBuffer());
            }

            submitInfo.pCommandBuffers = vkCommandBuffers.data();
            submitInfo.commandBufferCount = vkCommandBuffers.size();

            eastl::vector<VkSemaphore> waitSemaphores = {};
            eastl::vector<u64> waitSemaphoreValues = {};
            eastl::vector<VkSemaphore> signalSemaphores = {};
            eastl::vector<u64> signalSemaphoreValues = {};

            signalSemaphores.push_back(static_cast<VulkanFence*>(mMainQueueGpuFence)->GetVkSemaphore());
            signalSemaphoreValues.push_back(currentMainQueueCpuTimelineValue);

            for (auto [semaphore, index] : info.signalFences) {
                signalSemaphores.push_back(static_cast<VulkanFence*>(semaphore)->GetVkSemaphore());
                signalSemaphoreValues.push_back(index);
            }
            for (auto semaphore : info.signalSemaphores) {
                signalSemaphores.push_back(eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore());
                signalSemaphoreValues.push_back(0);
            }
            // TODO for Lukas, add timeline semaphores here
            for (auto semaphore : vkQueue->RefSubmittedSwapAcquireSemaphores()) {
                waitSemaphores.push_back(semaphore);
                waitSemaphoreValues.push_back(0);
            }

            const auto waitStageMasks = eastl::vector<VkPipelineStageFlags>(
                signalSemaphores.size(), /*FIXME: is this efficient? -> */ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);


            VkTimelineSemaphoreSubmitInfo timelineInfo{
                .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreValueCount = static_cast<u32>(waitSemaphoreValues.size()),
                .pWaitSemaphoreValues = waitSemaphoreValues.data(),
                .signalSemaphoreValueCount = static_cast<u32>(signalSemaphoreValues.size()),
                .pSignalSemaphoreValues = signalSemaphoreValues.data(),
            };

            submitInfo.pNext = &timelineInfo;
            submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphores = waitSemaphores.data();
            submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphores = signalSemaphores.data();
            submitInfo.pWaitDstStageMask = waitStageMasks.data();

            vkQueueSubmit(vkQueue->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);

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
                mCommandBufferPool->PutBack({ zombie.vkCmdPool, zombie.vkCmdBuffer });
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