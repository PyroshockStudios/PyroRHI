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

#include <RHIVulkan/VkContext.hpp>

#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        PYRO_FORCEINLINE static constexpr VkImageViewType ToVkImageViewType(ImageViewType type) { return static_cast<VkImageViewType>(type); }
        PYRO_FORCEINLINE static constexpr VkGeometryFlagsKHR ToVkGeometryFlagsKHR(AccelerationStructureGeometryFlags type) { return static_cast<VkGeometryFlagsKHR>(type.data); }

        VulkanDevice::VulkanDevice(VulkanContext* context, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& features, bool bHeadlessEnabled)
            : mContext(context), mPhysicalDevice(physicalDevice) {
            mFeatures.bHeadlessSwapChainWindow = bHeadlessEnabled;
            mInfo.bHeadless = bHeadlessEnabled;

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

            void* lastPhysicalDevicePnext = reinterpret_cast<void*>(&physicalDeviceFenceFeatures);

            eastl::vector<const char*> extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
            };

            u32 extensionCount;
            CheckVkResult(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr),
                "Failed to enumerate device extensions!");
            eastl::vector<VkExtensionProperties> availableExtensions(extensionCount);
            CheckVkResult(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data()),
                "Failed to enumerate device extensions!");

            auto tryEnableExtension = [physicalDevice, &lastPhysicalDevicePnext, &extensions](VkExtensionProperties& extension, const char* extensionName, auto& extensionFeatureStruct, const auto& checkFunc, const auto& customFunc) {
                if (strcmp(extension.extensionName, extensionName) == 0) {
                    VkPhysicalDeviceFeatures2 features2{
                        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                        .pNext = &extensionFeatureStruct,
                    };
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

                    if (checkFunc()) {
                        extensionFeatureStruct.pNext = lastPhysicalDevicePnext;
                        lastPhysicalDevicePnext = reinterpret_cast<void*>(&extensionFeatureStruct);

                        customFunc();

                        extensions.push_back(extension.extensionName);
                    }
                    return true;
                }
                return false;
            };

            VkPhysicalDeviceFeatures2 features2 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = nullptr,
            };
            vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);

            VkPhysicalDeviceLineRasterizationFeaturesEXT lineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT };
            VkPhysicalDeviceBufferDeviceAddressFeatures physicalDeviceBufferDeviceAddressFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
            VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
            VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
            VkPhysicalDeviceRayQueryFeaturesKHR physicalDeviceRayQueryFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
            VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR physicalDeviceRayTracingPositionFetchFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR };
            VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV physicalDeviceRayTracingInvocationReorderProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV };
            for (VkExtensionProperties& extension : availableExtensions) {
                tryEnableExtension(
                    extension, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME, lineFeatures,
                    [&]() {
                        return lineFeatures.smoothLines == VK_TRUE;
                    },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME " with 'smoothLines' is supported on this device.");
                        mVulkanCaps.bVK_EXT_line_rasterization = true;
                    });

                tryEnableExtension(
                    extension, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, physicalDeviceBufferDeviceAddressFeatures,
                    [&]() { return physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress == VK_TRUE; },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME " is supported on this device.");

                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddressCaptureReplay = VK_FALSE;
                        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddressMultiDevice = VK_FALSE;

                        mFeatures.bBufferDeviceAddress = true;
                        mVulkanCaps.bVK_KHR_buffer_device_address = true;
                    });

                tryEnableExtension(
                    extension, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, physicalDeviceAccelerationStructureFeatures,
                    [&]() { return physicalDeviceAccelerationStructureFeatures.accelerationStructure == VK_TRUE &&
                                   physicalDeviceAccelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind == VK_TRUE; },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME " is supported on this device.");

                        physicalDeviceAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
                        physicalDeviceAccelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE;
                        physicalDeviceAccelerationStructureFeatures.accelerationStructureCaptureReplay = VK_FALSE;
                        physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands = VK_FALSE;
                        physicalDeviceAccelerationStructureFeatures.accelerationStructureIndirectBuild = VK_FALSE;

                        mFeatures.bAccelerationStructureBuild = true;
                        mVulkanCaps.bVK_KHR_acceleration_structures = true;
                        // must also be supported
                        mVulkanCaps.bVK_KHR_deferred_host_operations = true;
                        extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
                    });

                tryEnableExtension(
                    extension, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, physicalDeviceRayTracingPipelineFeatures,
                    [&]() { return physicalDeviceRayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE; },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME " is supported on this device.");

                        physicalDeviceRayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE;
                        physicalDeviceRayTracingPipelineFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE;

                        mFeatures.bRayTracingPipelines = true;
                        mVulkanCaps.bVK_KHR_ray_tracing_pipeline = true;
                    });

                tryEnableExtension(
                    extension, VK_KHR_RAY_QUERY_EXTENSION_NAME, physicalDeviceRayQueryFeatures,
                    [&]() { return physicalDeviceRayQueryFeatures.rayQuery == VK_TRUE; },
                    [&]() {
                        physicalDeviceRayQueryFeatures.rayQuery = VK_TRUE;
                        Logger::Info(gVulkanSink, VK_KHR_RAY_QUERY_EXTENSION_NAME " is supported on this device.");
                        mFeatures.bRayQueries = true;
                        mVulkanCaps.bVK_KHR_ray_query = true;
                    });

                tryEnableExtension(
                    extension, VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, physicalDeviceRayTracingPositionFetchFeatures,
                    [&]() { return physicalDeviceRayTracingPositionFetchFeatures.rayTracingPositionFetch == VK_TRUE; },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME " is supported on this device.");
                        mVulkanCaps.bVK_KHR_ray_tracing_position_fetch = true;
                    });

                tryEnableExtension(
                    extension, VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME, physicalDeviceRayTracingInvocationReorderProperties,
                    [&]() { return physicalDeviceRayTracingInvocationReorderProperties.rayTracingInvocationReorderReorderingHint == VK_RAY_TRACING_INVOCATION_REORDER_MODE_REORDER_NV; },
                    [&]() {
                        Logger::Info(gVulkanSink, VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME " is supported on this device.");
                        mVulkanCaps.bVK_NV_ray_tracing_invocation_reorder = true;
                    });
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
                    .shaderInt64 = features.shaderInt64,
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

            CheckVkResult(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, mContext->GetVkAllocator(), &mDevice), "Failed to create vulkan device!");

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
                if (queue == VK_NULL_HANDLE) {
                    Logger::Fatal(gVulkanSink, "Faild to get command queue!");
                    return;
                }
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
            if (mVulkanCaps.bVK_KHR_buffer_device_address) {
                vmaAllocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            CheckVkResult(vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator), "Failed to create memory allocator!");

            mResourceTable.Initialize(MAX_VK_BINDLESS_BUFFERS, MAX_VK_BINDLESS_IMAGES, MAX_VK_BINDLESS_SAMPLERS,
                MAX_VK_VIRTUAL_MEMORIES, mVulkanCaps.bVK_KHR_acceleration_structures ? MAX_VK_ACCELERATION_STRUCTURES : 0,
                mDevice, mContext->GetVkAllocator(), VK_NULL_HANDLE, vkSetDebugUtilsObjectNameEXT);

            vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);

            PopulateDeviceInfo();
            PopulateDeviceProperties();
            PopulateDeviceFeatures();
        }

        VulkanDevice::~VulkanDevice() {
            VulkanDevice::WaitIdle();
            VulkanDevice::CollectGarbage();
            ASSERT(mResourceZombies.Empty(), "Dangling zombies remaining after device destruction! This should never happen!");
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
            CheckVkResult(vmaCreateVirtualBlock(&blockCreateInfo, &ret.vmaBlock), "Failed to create virtual memory block!");
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
            CheckVkResult(vmaAllocateMemory(mVmaAllocator, &ret.requirements, &vmaAllocationCreateInfo, &ret.vmaAllocation, &ret.vmaAllocationInfo), "Failed to allocate memory block!");
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
            ASSERT(info.usage.data != 0, "Buffers must have usage flags defined!");

            // NOTE:
            // Zilver - Since vulkan changes certain requirements for memory alignment,
            // I think it still makes sense to add the flags, despite the drivers usually ignoring it for the most part.
            VkBufferUsageFlags VK_BUFFER_USAGE_FLAGS = {};
            if (info.usage & BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS && mVulkanCaps.bVK_KHR_buffer_device_address) {
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
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT /*Clear UAV usage*/;
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

            if (info.usage & BufferUsageFlagBits::ACCELERATION_STRUCTURE) {
                ASSERT(mFeatures.bAccelerationStructureBuild, "Cannot create a buffer with ACCELERATION_STRUCTURE_BUFFER if the device does not support AccelerationStructureBuild!");
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (info.usage & BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER) {
                ASSERT(mFeatures.bAccelerationStructureBuild, "Cannot create a buffer with BLAS_GEOMETRY_BUFFER if the device does not support AccelerationStructureBuild!");
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            }
            if (info.usage & BufferUsageFlagBits::BLAS_INSTANCE_BUFFER) {
                ASSERT(mFeatures.bAccelerationStructureBuild, "Cannot create a buffer with BLAS_GEOMETRY_BUFFER if the device does not support AccelerationStructureBuild!");
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            }
            if (info.usage & BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER) {
                ASSERT(mFeatures.bAccelerationStructureBuild, "Cannot create a buffer with TLAS_BLAS_SCRATCH_BUFFER if the device does not support AccelerationStructureBuild!");
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
                VK_BUFFER_USAGE_FLAGS |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
                CheckVkResult(result, "Failed to create buffer!");
                ret.allocationInfo = vmaAllocationInfo;
            } else {
                auto& blockInfo = Slot(info.memoryBlock);
                CheckVkResult(vkCreateBuffer(mDevice, &vkBufferCreateInfo, mContext->GetVkAllocator(), &ret.vkBuffer),
                    "Failed to create buffer!");
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
                VkResult allocateResult = vmaVirtualAllocate(blockInfo.vmaBlock, &vmaVirtualAllocationCreateInfo, &ret.vmaAllocation.Get<VmaVirtualAllocation>(), &offset);
                if (allocateResult == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                    Logger::Debug(gVulkanSink, "Memory block {} of size {} ran out of space to allocate buffer {} with size {}. This is not an error",
                        blockInfo.info.name, blockInfo.info.size, info.name, info.size);
                    vkDestroyBuffer(mDevice, ret.vkBuffer, mContext->GetVkAllocator());
                    mResourceTable.mBufferSlots.ReturnSlot(id);
                    return PYRO_NULL_BUFFER;
                }
                CheckVkResult(vkBindBufferMemory(mDevice, ret.vkBuffer, blockInfo.vmaAllocationInfo.deviceMemory, offset + blockInfo.vmaAllocationInfo.offset),
                    "Failed to bind buffer memory!");
                vmaGetVirtualAllocationInfo(blockInfo.vmaBlock, ret.vmaAllocation.Get<VmaVirtualAllocation>(), &ret.allocationInfo.Get<VmaVirtualAllocationInfo>());
                ++blockInfo.debugReferences;
            }
            const VkBufferDeviceAddressInfo vkBufferDeviceAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .pNext = nullptr,
                .buffer = ret.vkBuffer,
            };
            if ((VK_BUFFER_USAGE_FLAGS & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) && mVulkanCaps.bVK_KHR_buffer_device_address) {
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
            ASSERT(info.usage.data != 0, "Images must have usage flags defined!");

            VkImageCreateInfo vkImageCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .format = ToVkFormat(info.format),
                .extent = { info.size.width, info.size.height, info.size.depth },
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
            // if (info.arrayLayerCount > 1 && info.dimensions == ImageDimensions::e2D) {
            //     vkImageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
            // }
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
            case ImageDimensions::e1D:
                vkImageCreateInfo.imageType = VK_IMAGE_TYPE_1D;
                break;
            case ImageDimensions::e2D:
                vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                break;
            case ImageDimensions::e3D:
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
                CheckVkResult(result, "Failed to create image!");
                ret.allocationInfo = vmaAllocationInfo;
            } else {
                auto& blockInfo = Slot(info.memoryBlock);
                CheckVkResult(vkCreateImage(mDevice, &vkImageCreateInfo, mContext->GetVkAllocator(), &ret.vkImage),
                    "Failed to craete image!");
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
                VkResult allocateResult = vmaVirtualAllocate(blockInfo.vmaBlock, &vmaVirtualAllocationCreateInfo, &ret.vmaAllocation.Get<VmaVirtualAllocation>(), &offset);
                if (allocateResult == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                    Logger::Debug(gVulkanSink, "Memory block {} of size {} ran out of space to allocate image {} with {} bytes. This is not an error",
                        blockInfo.info.name, blockInfo.info.size, info.name, requirements.size);
                    vkDestroyImage(mDevice, ret.vkImage, mContext->GetVkAllocator());
                    mResourceTable.mImageSlots.ReturnSlot(id);
                    return PYRO_NULL_IMAGE;
                }

                CheckVkResult(vkBindImageMemory(mDevice, ret.vkImage, blockInfo.vmaAllocationInfo.deviceMemory, offset + blockInfo.vmaAllocationInfo.offset),
                    "Failed to bind image memory!");
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

        ShaderResourceId VulkanDevice::CreateShaderResource(const GpuResourceInfo& info) {
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                return ShaderResourceId{ CreateBufferView(info) };
            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                return ShaderResourceId{ CreateImageView(info, false) };
            } else {
                ASSERT(false, "Bad variant");
            }
            return ShaderResourceId{};
        }

        UnorderedAccessId VulkanDevice::CreateUnorderedAccess(const GpuResourceInfo& info) {
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                return UnorderedAccessId{ CreateBufferView(info) };
            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                return UnorderedAccessId{ CreateImageView(info, true) };
            } else {
                ASSERT(false, "Bad variant");
            }
            return UnorderedAccessId{};
        }

        GpuResourceId VulkanDevice::CreateBufferView(const GpuResourceInfo& info) {
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

        void VulkanDevice::PopulateDeviceInfo() {
            const VkPhysicalDeviceProperties& props = mPhysicalDeviceProperties;

            mInfo.name = props.deviceName;
            mInfo.vendorID = props.vendorID;
            mInfo.deviceID = props.deviceID;
            mInfo.apiVersion.sprintf("Vulkan %d.%d.%d",
                VK_API_VERSION_MAJOR(props.apiVersion),
                VK_API_VERSION_MINOR(props.apiVersion),
                VK_API_VERSION_PATCH(props.apiVersion));

            switch (props.vendorID) {
            case 0x10DE:
                mInfo.vendor = "NVIDIA";
                break;
            case 0x1002:
            case 0x1022:
                mInfo.vendor = "AMD";
                break;
            case 0x8086:
                mInfo.vendor = "Intel";
                break;
            case 0x13B5:
                mInfo.vendor = "ARM";
                break;
            case 0x5143:
                mInfo.vendor = "Qualcomm";
                break;
            case 0x1414:
                mInfo.vendor = "Microsoft";
                break;
            default:
                mInfo.vendor = "Unknown";
                break;
            }

            mInfo.driverVersion.sprintf("0x%X", props.driverVersion);

            VkPhysicalDeviceMemoryProperties memProps = {};
            vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProps);

            mInfo.bUnifiedMemory = false;
            for (u32 i = 0; i < memProps.memoryHeapCount; ++i) {
                if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    mInfo.dedicatedVideoMemory += memProps.memoryHeaps[i].size;
                } else {
                    mInfo.sharedSystemMemory += memProps.memoryHeaps[i].size;
                }
            }

            switch (props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                mInfo.deviceType = DeviceType::Integrated;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                mInfo.deviceType = DeviceType::Discrete;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                mInfo.deviceType = DeviceType::Virtual;
                mInfo.bRemovable = true;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                mInfo.deviceType = DeviceType::CPU;
                break;
            default:
                mInfo.deviceType = DeviceType::Unknown;
                break;
            }
            mInfo.bHeadless = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);
            u32 count = 1;
            VkPhysicalDevice physd = VK_NULL_HANDLE;
            vkEnumeratePhysicalDevices(mContext->GetVkInstance(), &count, &physd);
            mInfo.bPrimaryAdapter = physd == mPhysicalDevice;
        }

        void VulkanDevice::PopulateDeviceProperties() {
            const VkPhysicalDeviceProperties& props = mPhysicalDeviceProperties;

            VkSampleCountFlags colorTargetSampleCounts = props.limits.framebufferColorSampleCounts;
            VkSampleCountFlags depthSampleCounts = props.limits.framebufferDepthSampleCounts;
            VkSampleCountFlags stencilSampleCounts = props.limits.framebufferStencilSampleCounts;
            VkSampleCountFlags countsDepthStencil = depthSampleCounts & stencilSampleCounts;
            VkSampleCountFlags sampledColorSampleCounts = props.limits.sampledImageColorSampleCounts;
            VkSampleCountFlags sampledDepthSampleCounts = props.limits.sampledImageDepthSampleCounts;
            VkSampleCountFlags sampledIntegerSampleCounts = props.limits.sampledImageIntegerSampleCounts;
            VkSampleCountFlags sampledStencilSampleCounts = props.limits.sampledImageStencilSampleCounts;
            VkSampleCountFlags countsSampled = sampledColorSampleCounts & sampledDepthSampleCounts & sampledIntegerSampleCounts & sampledStencilSampleCounts;

            VkSampleCountFlags storageSampleCounts = props.limits.storageImageSampleCounts;
            mProperties.msaaSupportColorTarget = static_cast<RasterizationSamples>(colorTargetSampleCounts);
            mProperties.msaaSupportDepthStencilTarget = static_cast<RasterizationSamples>(countsDepthStencil);
            mProperties.msaaSupportShaderResourceView = static_cast<RasterizationSamples>(countsSampled);
            mProperties.msaaSupportUnorderedAccessView = static_cast<RasterizationSamples>(storageSampleCounts);

            const auto& limits = props.limits;
            mProperties.bufferImageRowAlignment = limits.optimalBufferCopyRowPitchAlignment;
            mProperties.bufferImageCopyOffsetAlignment = limits.optimalBufferCopyOffsetAlignment;
            mProperties.minUniformBufferOffsetAlignment = limits.minUniformBufferOffsetAlignment;
            mProperties.minStorageBufferOffsetAlignment = limits.minStorageBufferOffsetAlignment;

            mProperties.maxTextureWidth = limits.maxImageDimension2D;
            mProperties.maxTextureHeight = limits.maxImageDimension2D;
            mProperties.maxTextureDepth = limits.maxImageDimension3D;
            mProperties.maxTextureArrayLayers = limits.maxImageArrayLayers;
            mProperties.maxSamplerAnisotropy = static_cast<u32>(limits.maxSamplerAnisotropy);

            mProperties.minLineWidth = limits.lineWidthRange[0];
            mProperties.maxLineWidth = limits.lineWidthRange[1];

            for (ICommandQueue* q : mCommandQueues) {
                if (q->Info().flags & CommandQueueFlagBits::COMPUTE && !(q->Info().flags & CommandQueueFlagBits::GRAPHICS)) {
                    mProperties.bHasDedicatedComputeQueue = true;
                } else if (q->Info().flags & CommandQueueFlagBits::TRANSFER && !(q->Info().flags & CommandQueueFlagBits::GRAPHICS)) {
                    mProperties.bHasDedicatedTransferQueue = true;
                }
                if (q->Info().flags & CommandQueueFlagBits::GRAPHICS) {
                    ++mProperties.graphicsQueueCount;
                }
                if (q->Info().flags & CommandQueueFlagBits::COMPUTE) {
                    ++mProperties.computeQueueCount;
                }
                if (q->Info().flags & CommandQueueFlagBits::TRANSFER) {
                    ++mProperties.transferQueueCount;
                }
            }
        }

        void VulkanDevice::PopulateDeviceFeatures() {
            VkPhysicalDeviceFeatures features = {};
            vkGetPhysicalDeviceFeatures(mPhysicalDevice, &features);
            const VkPhysicalDeviceProperties& props = mPhysicalDeviceProperties;

            mFeatures.bBCnTextureCompression = features.textureCompressionBC;
            mFeatures.bGeometryShaders = features.geometryShader;
            mFeatures.bTesselationShaders = features.tessellationShader;
            mFeatures.bInt64ShaderOps = features.shaderInt64;
            mFeatures.bVariableRateShading = false;
            // FIXME: this is not a good way to do, do with VkPhysicalDeviceSubgroupProperties
            mFeatures.bWaveOps = features.shaderInt64;


            // props.apiVersion = VK_MAKE_VERSION(major, minor, patch)
            uint32_t vulkanVersion = props.apiVersion;

            uint32_t major = VK_VERSION_MAJOR(vulkanVersion);
            uint32_t minor = VK_VERSION_MINOR(vulkanVersion);


            static const auto VulkanVersionAtLeast = [](uint32_t majorV, uint32_t minorV, uint32_t major, uint32_t minor) {
                return (major > majorV) || (major == majorV && minor >= minorV);
            };

            // Infer SPIR-V version as RHI numeric code
            if (VulkanVersionAtLeast(1, 4, major, minor)) {
                mFeatures.maxSupportedShaderModel = 0x16; // SPIR-V 1.6
            } else if (VulkanVersionAtLeast(1, 3, major, minor)) {
                mFeatures.maxSupportedShaderModel = 0x15; // SPIR-V 1.5
            } else if (VulkanVersionAtLeast(1, 1, major, minor)) {
                // Query optional extension for SPIR-V 1.4
                uint32_t extCount = 0;
                vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extCount, nullptr);
                std::vector<VkExtensionProperties> extensions(extCount);
                vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extCount, extensions.data());

                bool bSupportsSPIRV14 = false;
                for (const auto& ext : extensions) {
                    if (strcmp(ext.extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0) {
                        bSupportsSPIRV14 = true;
                        break;
                    }
                }
                mFeatures.maxSupportedShaderModel = bSupportsSPIRV14 ? 0x14 : 0x13; // SPIR-V 1.4 or 1.3
            } else {
                mFeatures.maxSupportedShaderModel = 0x10; // SPIR-V 1.0
            }
        }

        GpuResourceId VulkanDevice::CreateImageView(const GpuResourceInfo& info, bool uav) {
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
            CheckVkResult(result, "Failed to create image view!");

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
            CheckVkResult(result, "Failed to create sampler object!");

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

        BlasId VulkanDevice::CreateBlas(const BlasInfo& info) {
            auto [id, ret] = mResourceTable.mBlasSlots.NewSlot();
            ret.info = info;

            ret.ownsBuffer = true;
            ret.bufferId = CreateBuffer({ .size = info.size,
                .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE,
                .allocationDomain = MemoryAllocationDomain::DeviceLocal,
                .name = info.name + " - buffer" });
            ret.vkBuffer = Slot(ret.bufferId).vkBuffer;

            VkAccelerationStructureCreateInfoKHR vkCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
                .pNext = nullptr,
                .createFlags = {}, // VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
                .buffer = Slot(ret.bufferId).vkBuffer,
                .offset = ret.offset,
                .size = ret.info.size,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .deviceAddress = {},
            };
            VkResult result = vkCreateAccelerationStructureKHR(mDevice, &vkCreateInfo, mContext->GetVkAllocator(), &ret.vkAccelerationStructure);
            CheckVkResult(result, "Failed to create acceleration structure (BLAS)!");

            VkAccelerationStructureDeviceAddressInfoKHR vkAccelerationStructureDeviceAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
                .pNext = nullptr,
                .accelerationStructure = ret.vkAccelerationStructure,
            };

            ret.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(mDevice, &vkAccelerationStructureDeviceAddressInfo);

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT blasNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vkAccelerationStructure),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &blasNameInfo);
            }

            return BlasId{ id };
        }

        TlasId VulkanDevice::CreateTlas(const TlasInfo& info) {
            auto [id, ret] = mResourceTable.mTlasSlots.NewSlot();
            ret.info = info;

            ret.ownsBuffer = true;
            ret.bufferId = CreateBuffer({ .size = info.size,
                .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE,
                .allocationDomain = MemoryAllocationDomain::DeviceLocal,
                .name = info.name + " - buffer" });
            ret.vkBuffer = Slot(ret.bufferId).vkBuffer;

            VkAccelerationStructureCreateInfoKHR vkCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
                .pNext = nullptr,
                .createFlags = {}, // VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
                .buffer = Slot(ret.bufferId).vkBuffer,
                .offset = ret.offset,
                .size = ret.info.size,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                .deviceAddress = {},
            };
            VkResult result = vkCreateAccelerationStructureKHR(mDevice, &vkCreateInfo, mContext->GetVkAllocator(), &ret.vkAccelerationStructure);
            CheckVkResult(result, "Failed to create acceleration structure (TLAS)!");

            VkAccelerationStructureDeviceAddressInfoKHR vkAccelerationStructureDeviceAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
                .pNext = nullptr,
                .accelerationStructure = ret.vkAccelerationStructure,
            };

            ret.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(mDevice, &vkAccelerationStructureDeviceAddressInfo);

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT tlasNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR,
                    .objectHandle = eastl::bit_cast<uint64_t>(ret.vkAccelerationStructure),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice, &tlasNameInfo);
            }

            mResourceTable.WriteDescriptorSetAccelerationStructure(mDevice, mResourceTable.mBindlessDescriptorSet, ret.vkAccelerationStructure, id.index);

            return TlasId{ id };
        }


        const MemoryBlockInfo& VulkanDevice::GetMemoryBlockInfo(MemoryBlock memory) const {
            return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(memory)).info;
        }

        const BufferInfo& VulkanDevice::GetBufferInfo(Buffer buffer) const {
            return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(buffer)).info;
        }

        const ImageInfo& VulkanDevice::GetImageInfo(Image image) const {
            return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(image)).info;
        }

        const GpuResourceInfo& VulkanDevice::GetShaderResourceInfo(ShaderResourceId id) const {
            return mResourceTable.mResourceViewSlots.DereferenceId(id).info;
        }
        const GpuResourceInfo& VulkanDevice::GetUnorderedAccessInfo(UnorderedAccessId id) const {
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

        const BlasInfo& VulkanDevice::GetBlasInfo(BlasId blas) const {
            return mResourceTable.mBlasSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(blas)).info;
        }

        const TlasInfo& VulkanDevice::GetTlasInfo(TlasId tlas) const {
            return mResourceTable.mTlasSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(tlas)).info;
        }

        DeviceAddress VulkanDevice::BufferDeviceAddress(Buffer buffer) const {
            return DeviceAddress();
        }

        u8* VulkanDevice::BufferHostAddress(Buffer buffer) const {
            return reinterpret_cast<u8*>(Slot(buffer).hostAddress);
        }

        BlasAddress VulkanDevice::BlasInstanceAddress(BlasId blas) const {
            return static_cast<BlasAddress>(Slot(blas).deviceAddress);
        }


        DeviceSize VulkanDevice::ImageSizeRequirements(Image image) const {
            auto& img = Slot(image);
            VkMemoryRequirements requirements;
            vkGetImageMemoryRequirements(mDevice, img.vkImage, &requirements);
            return requirements.size;
        }
        u32 VulkanDevice::ImageSubresourceRowPitch(Image image, u32 rowWidth, ImageSlice slice) const {
            return PYRO_ALIGN(rowWidth, mPhysicalDeviceProperties.limits.optimalBufferCopyRowPitchAlignment);
        }


        void VulkanDevice::CreateAccelerationStructureBuildInfo(
            const eastl::span<const TlasBuildInfo>& tlasBuildInfos, const eastl::span<const BlasBuildInfo>& blasBuildInfos,
            eastl::vector<VkAccelerationStructureBuildGeometryInfoKHR>& vkBuildGeometryInfos,
            eastl::vector<VkAccelerationStructureGeometryKHR>& vkGeometryInfos,
            eastl::vector<u32>& primitiveCounts,
            eastl::vector<const u32*>& primitiveCountsPtrs) const {
            vkBuildGeometryInfos.reserve(blasBuildInfos.size() + tlasBuildInfos.size());
            primitiveCounts.reserve(blasBuildInfos.size() + tlasBuildInfos.size());
            usize geometryInfoCount = 0;
            for (const auto& tlasBuildInfo : tlasBuildInfos) {
                geometryInfoCount += 1;
            }
            for (const auto& blasBuildInfo : blasBuildInfos) {
                if (auto* triangleGeometryInfos = eastl::get_if<eastl::span<const BlasTriangleGeometryInfo>>(&blasBuildInfo.geometries)) {
                    geometryInfoCount += triangleGeometryInfos->size();
                }

                if (auto* aabbGeometryInfos = eastl::get_if<eastl::span<const BlasAabbGeometryInfo>>(&blasBuildInfo.geometries)) {
                    geometryInfoCount += aabbGeometryInfos->size();
                }
            }
            vkGeometryInfos.reserve(geometryInfoCount);
            primitiveCounts.reserve(geometryInfoCount);
            primitiveCountsPtrs.reserve(geometryInfoCount);

            for (const auto& blasBuildInfo : blasBuildInfos) {
                const VkAccelerationStructureGeometryKHR* vkGeometryArrayPtr = vkGeometryInfos.data() + vkGeometryInfos.size();
                const u32* primitiveCountsPtr = primitiveCounts.data() + primitiveCounts.size();

                u32 geometryCount = 0;
                if (auto* triangleGeometryInfos = eastl::get_if<eastl::span<const BlasTriangleGeometryInfo>>(&blasBuildInfo.geometries)) {
                    geometryCount = triangleGeometryInfos->size();
                    for (const auto& geometry : *triangleGeometryInfos) {
                        ASSERT(geometry.vertexBuffer != PYRO_NULL_BUFFER, "Vertex buffer must never be null when creating a triangle BLAS!");
                        VkAccelerationStructureGeometryKHR geometryInfo = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                            .pNext = nullptr,
                            .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                            .geometry = {
                                .triangles = {
                                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                                    .pNext = nullptr,
                                    .vertexFormat = ToVkFormat(geometry.vertexFormat),
                                    .vertexData = eastl::bit_cast<VkDeviceOrHostAddressConstKHR>(Slot(geometry.vertexBuffer).deviceAddress + geometry.vertexByteOffset),
                                    .vertexStride = geometry.vertexStride,
                                    .maxVertex = geometry.vertexCount - 1,
                                    .indexType = ToVkIndexType(geometry.indexType),
                                    .indexData = geometry.indexBuffer == PYRO_NULL_BUFFER
                                                     ? VkDeviceOrHostAddressConstKHR{}
                                                     : eastl::bit_cast<VkDeviceOrHostAddressConstKHR>(Slot(geometry.indexBuffer).deviceAddress + geometry.indexOffset * ToVkIndexTypeSize(geometry.indexType)),
                                    .transformData = geometry.transformData == PYRO_NULL_BUFFER
                                                         ? VkDeviceOrHostAddressConstKHR{}
                                                         : eastl::bit_cast<VkDeviceOrHostAddressConstKHR>(Slot(geometry.transformData).deviceAddress + geometry.transformDataOffset),
                                },
                            },
                            .flags = ToVkGeometryFlagsKHR(geometry.flags),
                        };
                        primitiveCounts.push_back(geometry.indexBuffer == PYRO_NULL_BUFFER ? geometry.vertexCount / 3 : geometry.indexCount / 3);
                        vkGeometryInfos.push_back(geometryInfo);
                    }
                }

                if (auto* aabbGeometryInfos = eastl::get_if<eastl::span<const BlasAabbGeometryInfo>>(&blasBuildInfo.geometries)) {
                    geometryCount = aabbGeometryInfos->size();
                    for (const auto& geometry : *aabbGeometryInfos) {
                        ASSERT(geometry.data != PYRO_NULL_BUFFER, "AABB data buffer must never be null when creating a AABB BLAS!");
                        VkAccelerationStructureGeometryKHR geometryInfo = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                            .pNext = nullptr,
                            .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
                            .geometry = {
                                .aabbs = {
                                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                                    .pNext = nullptr,
                                    .data = eastl::bit_cast<VkDeviceOrHostAddressConstKHR>(Slot(geometry.data).deviceAddress),
                                    .stride = geometry.stride,
                                },
                            },
                            .flags = eastl::bit_cast<VkGeometryFlagsKHR>(geometry.flags),
                        };
                        primitiveCounts.push_back(geometry.count);
                        vkGeometryInfos.push_back(geometryInfo);
                    }
                }

                vkBuildGeometryInfos.push_back({
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                    .pNext = nullptr,
                    .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                    .flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(blasBuildInfo.flags),
                    .mode = blasBuildInfo.bUpdate ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                    .srcAccelerationStructure = blasBuildInfo.srcBlas != PYRO_NULL_BLAS ? Slot(blasBuildInfo.srcBlas).vkAccelerationStructure : VK_NULL_HANDLE,
                    .dstAccelerationStructure = blasBuildInfo.dstBlas != PYRO_NULL_BLAS ? Slot(blasBuildInfo.dstBlas).vkAccelerationStructure : VK_NULL_HANDLE,
                    .geometryCount = geometryCount,
                    .pGeometries = vkGeometryArrayPtr,
                    .ppGeometries = nullptr,
                    .scratchData = blasBuildInfo.scratchBuffer == PYRO_NULL_BUFFER
                                       ? VkDeviceOrHostAddressKHR{}
                                       : eastl::bit_cast<VkDeviceOrHostAddressKHR>(Slot(blasBuildInfo.scratchBuffer).deviceAddress),
                });
                primitiveCountsPtrs.push_back(primitiveCountsPtr);
            }
            for (const auto& tlasBuildInfo : tlasBuildInfos) {
                const VkAccelerationStructureGeometryKHR* vkGeometryArrayPtr = vkGeometryInfos.data() + vkGeometryInfos.size();
                const u32* primitiveCountsPtr = primitiveCounts.data() + primitiveCounts.size();

                VkAccelerationStructureGeometryInstancesDataKHR vkInstanceData = {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                    .pNext = nullptr,
                    .arrayOfPointers = /* tlasBuildInfo.instances.bDataArrayOfPointers ? VK_TRUE :*/ VK_FALSE,
                    .data = eastl::bit_cast<VkDeviceOrHostAddressConstKHR>(Slot(tlasBuildInfo.instances.data).deviceAddress)
                };
                vkGeometryInfos.push_back(VkAccelerationStructureGeometryKHR{
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .pNext = nullptr,
                    .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                    .geometry = VkAccelerationStructureGeometryDataKHR{
                        .instances = vkInstanceData } });
                primitiveCounts.push_back(tlasBuildInfo.instances.count);

                vkBuildGeometryInfos.push_back({
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                    .pNext = nullptr,
                    .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                    .flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(tlasBuildInfo.flags),
                    .mode = tlasBuildInfo.update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                    .srcAccelerationStructure = tlasBuildInfo.srcTlas != PYRO_NULL_TLAS ? Slot(tlasBuildInfo.srcTlas).vkAccelerationStructure : nullptr,
                    .dstAccelerationStructure = tlasBuildInfo.dstTlas != PYRO_NULL_TLAS ? Slot(tlasBuildInfo.dstTlas).vkAccelerationStructure : nullptr,
                    .geometryCount = 1,
                    .pGeometries = vkGeometryArrayPtr,
                    .ppGeometries = nullptr,
                    .scratchData = tlasBuildInfo.scratchBuffer == PYRO_NULL_BUFFER
                                       ? VkDeviceOrHostAddressKHR{}
                                       : eastl::bit_cast<VkDeviceOrHostAddressKHR>(Slot(tlasBuildInfo.scratchBuffer).deviceAddress),

                });
                primitiveCountsPtrs.push_back(primitiveCountsPtr);
            }
        }

        AccelerationStructureBuildSizesInfo VulkanDevice::BlasSizeRequirements(const BlasBuildInfo& info) const {
            eastl::vector<VkAccelerationStructureBuildGeometryInfoKHR> vkBuildGeometryInfos = {};
            eastl::vector<VkAccelerationStructureGeometryKHR> vkGeometryInfos = {};
            eastl::vector<u32> primitiveCounts = {};
            eastl::vector<const u32*> primitiveCountsPtrs = {};
            CreateAccelerationStructureBuildInfo({}, { &info, 1 }, vkBuildGeometryInfos, vkGeometryInfos, primitiveCounts, primitiveCountsPtrs);

            VkAccelerationStructureBuildSizesInfoKHR vkAccelerationStructureBuildSizesInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
                .pNext = nullptr,
            };

            vkGetAccelerationStructureBuildSizesKHR(mDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, vkBuildGeometryInfos.data(), primitiveCounts.data(), &vkAccelerationStructureBuildSizesInfo);

            return {
                .accelerationStructureSize = vkAccelerationStructureBuildSizesInfo.accelerationStructureSize,
                .updateScratchSize = vkAccelerationStructureBuildSizesInfo.updateScratchSize,
                .buildScratchSize = vkAccelerationStructureBuildSizesInfo.buildScratchSize,
            };
        }

        AccelerationStructureBuildSizesInfo VulkanDevice::TlasSizeRequirements(const TlasBuildInfo& info) const {
            eastl::vector<VkAccelerationStructureBuildGeometryInfoKHR> vkBuildGeometryInfos = {};
            eastl::vector<VkAccelerationStructureGeometryKHR> vkGeometryInfos = {};
            eastl::vector<u32> primitiveCounts = {};
            eastl::vector<const u32*> primitiveCountsPtrs = {};
            CreateAccelerationStructureBuildInfo({ &info, 1 }, {}, vkBuildGeometryInfos, vkGeometryInfos, primitiveCounts, primitiveCountsPtrs);

            VkAccelerationStructureBuildSizesInfoKHR vkAccelerationStructureBuildSizesInfo = {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
                .pNext = nullptr,
            };

            vkGetAccelerationStructureBuildSizesKHR(mDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, vkBuildGeometryInfos.data(), primitiveCounts.data(), &vkAccelerationStructureBuildSizesInfo);

            return {
                .accelerationStructureSize = vkAccelerationStructureBuildSizesInfo.accelerationStructureSize,
                .updateScratchSize = vkAccelerationStructureBuildSizesInfo.updateScratchSize,
                .buildScratchSize = vkAccelerationStructureBuildSizesInfo.buildScratchSize,
            };
        }

        bool VulkanDevice::IsMemoryBlockValid(MemoryBlock memory) const {
            return mResourceTable.mVirtualBlockSlots.IsIdValid(eastl::bit_cast<GpuResourceId>(memory));
        }

        bool VulkanDevice::IsBufferValid(Buffer buffer) const {
            return mResourceTable.mBufferSlots.IsIdValid(eastl::bit_cast<GpuResourceId>(buffer));
        }

        bool VulkanDevice::IsImageValid(Image image) const {
            return mResourceTable.mImageSlots.IsIdValid(eastl::bit_cast<GpuResourceId>(image));
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

        bool VulkanDevice::IsBlasValid(BlasId id) const {
            return mResourceTable.mBlasSlots.IsIdValid(id);
        }

        bool VulkanDevice::IsTlasValid(TlasId id) const {
            return mResourceTable.mTlasSlots.IsIdValid(id);
        }

        void VulkanDevice::DestroyRenderTarget(RenderTarget& renderTarget, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(renderTarget),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<RenderTarget>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }
            VulkanRenderTarget* targ = eastl::bit_cast<VulkanRenderTarget*>(renderTarget);
            delete targ;
            renderTarget = nullptr;
        }
        void VulkanDevice::DestroyRasterPipeline(RasterPipeline& pipeline, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(pipeline),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<RasterPipeline>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }
            delete eastl::bit_cast<VulkanRasterPipeline*>(pipeline);
            pipeline = nullptr;
        }
        void VulkanDevice::DestroyComputePipeline(ComputePipeline& pipeline, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(pipeline),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<ComputePipeline>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }
            delete eastl::bit_cast<VulkanComputePipeline*>(pipeline);
            pipeline = nullptr;
        }
        void VulkanDevice::DestroySwapChain(ISwapChain*& swapChain, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(swapChain),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<ISwapChain*>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ASSERT(dynamic_cast<VulkanSwapChain*>(swapChain) != nullptr, "Must be of type VulkanSwapChain!");
            delete static_cast<VulkanSwapChain*>(swapChain);
            swapChain = nullptr;
        }

        void VulkanDevice::DestroyMemoryBlock(MemoryBlock& memory, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(memory),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<MemoryBlock>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }
            ImplVmaVirtualBlockSlot& blockSlot = Slot(memory);
            ASSERT(blockSlot.debugReferences == 0, "Not all references to this MemoryBlock have been freed yet!");
            vmaFreeMemory(mVmaAllocator, blockSlot.vmaAllocation);
            vmaDestroyVirtualBlock(blockSlot.vmaBlock);
            blockSlot = {};
            mResourceTable.mVirtualBlockSlots.ReturnSlot(eastl::bit_cast<GpuResourceId>(memory));
            memory = PYRO_NULL_MEMORY_BLOCK;
        }

        void VulkanDevice::DestroyBuffer(Buffer& buffer, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(buffer),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<Buffer>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

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
            mResourceTable.mBufferSlots.ReturnSlot(eastl::bit_cast<GpuResourceId>(buffer));
            buffer = PYRO_NULL_BUFFER;
        }

        void VulkanDevice::DestroyImage(Image& image, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(image),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<Image>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

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
            mResourceTable.mImageSlots.ReturnSlot(eastl::bit_cast<GpuResourceId>(image));
            image = PYRO_NULL_IMAGE;
        }

        void VulkanDevice::DestroyShaderResource(ShaderResourceId& srv, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(srv),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<ShaderResourceId>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ImplResourceViewSlot& srvSlot = Slot(srv);
            if (eastl::holds_alternative<ImageResourceInfo>(srvSlot.info)) {
                vkDestroyImageView(mDevice, srvSlot.descriptor.Get<VkDescriptorImageInfo>().imageView, mContext->GetVkAllocator());
            }
            srvSlot = {};
            mResourceTable.mResourceViewSlots.ReturnSlot(srv);
            srv = PYRO_NULL_SRV;
        }
        void VulkanDevice::DestroyUnorderedAccess(UnorderedAccessId& uav, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(uav),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<UnorderedAccessId>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ImplResourceViewSlot& uavSlot = Slot(uav);
            if (eastl::holds_alternative<ImageResourceInfo>(uavSlot.info)) {
                vkDestroyImageView(mDevice, uavSlot.descriptor.Get<VkDescriptorImageInfo>().imageView, mContext->GetVkAllocator());
            }
            uavSlot = {};
            mResourceTable.mResourceViewSlots.ReturnSlot(uav);
            uav = PYRO_NULL_UAV;
        }

        void VulkanDevice::DestroySampler(SamplerId& sampler, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(sampler),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<SamplerId>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ImplSamplerSlot& samplerSlot = mResourceTable.mSamplerSlots.DereferenceId(sampler);
            vkDestroySampler(mDevice, samplerSlot.vkSampler, mContext->GetVkAllocator());
            samplerSlot = {};
            mResourceTable.mSamplerSlots.ReturnSlot(sampler);
            sampler = PYRO_NULL_SAMPLER;
        }

        void VulkanDevice::DestroySemaphore(Semaphore& semaphore, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(semaphore),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<Semaphore>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            VulkanSemaphore* sem = eastl::bit_cast<VulkanSemaphore*>(semaphore);
            delete sem;
            semaphore = nullptr;
        }

        void VulkanDevice::DestroyFence(IFence*& fence, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(fence),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<IFence*>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ASSERT(dynamic_cast<VulkanFence*>(fence) != nullptr, "Must be of type VulkanFence!");
            VulkanFence* fen = static_cast<VulkanFence*>(fence);
            delete fen;
            fence = nullptr;
        }

        void VulkanDevice::DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(queryPool),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<ITimestampQueryPool*>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            VulkanTimestampQueryPool* q = static_cast<VulkanTimestampQueryPool*>(queryPool);
            delete q;
            queryPool = nullptr;
        }

        void VulkanDevice::DestroyBlas(BlasId& blas, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(blas),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<BlasId>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ImplBlasSlot& blasSlot = mResourceTable.mBlasSlots.DereferenceId(blas);
            vkDestroyAccelerationStructureKHR(mDevice, blasSlot.vkAccelerationStructure, mContext->GetVkAllocator());

            if (blasSlot.ownsBuffer) {
                VulkanDevice::DestroyBuffer(blasSlot.bufferId, false);
            }

            blasSlot = {};
            mResourceTable.mBlasSlots.ReturnSlot(blas);
            blas = PYRO_NULL_BLAS;
        }

        void VulkanDevice::DestroyTlas(TlasId& tlas, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(tlas),
                    .deleter = [](VulkanDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<TlasId>(res)); }
                };
                mResourceZombies.EmplaceBack(eastl::move(SnapshotQueueTimelineValues()), zombie);
                return;
            }

            ImplTlasSlot& tlasSlot = mResourceTable.mTlasSlots.DereferenceId(tlas);
            vkDestroyAccelerationStructureKHR(mDevice, tlasSlot.vkAccelerationStructure, mContext->GetVkAllocator());

            if (tlasSlot.ownsBuffer) {
                VulkanDevice::DestroyBuffer(tlasSlot.bufferId, false);
            }

            tlasSlot = {};
            mResourceTable.mTlasSlots.ReturnSlot(tlas);
            tlas = PYRO_NULL_TLAS;
        }

        VulkanSwapChainSupportInfo VulkanDevice::GetSwapChainSupport(VkSurfaceKHR surface) const {
            VulkanSwapChainSupportInfo support{};
            VkBool32 bSupported = VK_FALSE;
            CheckVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, mPresentQueueFamilyIndex, surface, &bSupported), "Failed to query surface support!");
            if (bSupported == VK_FALSE) {
                return support; // no support whatsoever??
            }
            CheckVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, surface, &support.capabilities), "Failed to query windowing surface capabilities!");
            u32 formatCount = 0;
            CheckVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &formatCount, nullptr), "Failed to query windowing surface formats!");

            if (formatCount != 0) {
                support.formats.resize(formatCount);
                CheckVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &formatCount, support.formats.data()), "Failed to query windowing surface formats!");
            }

            u32 presentModeCount = 0;
            CheckVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, nullptr), "Failed to query windowing surface present modes!");

            if (presentModeCount != 0) {
                support.presentModes.resize(presentModeCount);
                CheckVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, support.presentModes.data()), "Failed to query windowing surface present modes!");
            }
            if (support.capabilities.maxImageCount == 0) { // llvmpipe what??
                support.capabilities.maxImageCount = UINT32_MAX;
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
                return {};
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
            // mActiveSwapChains.emplace(swapchain);
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
            CheckVkResult(result, "Failed to create semaphore!");

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
            CheckVkResult(result, "Failed to create fence!");

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

        eastl::optional<Format> VulkanDevice::PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) const {
            VkFormatFeatureFlags flags = ToVkFormatFeatureFlags(features);
            for (Format format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, ToVkFormat(format), &props);
                if ((props.optimalTilingFeatures & flags) == flags) {
                    return format;
                }
            }
            return eastl::nullopt;
        }

        void VulkanDevice::WaitIdle() {
            for (ICommandQueue* queue : mCommandQueues) {
                queue->WaitIdle();
            }
            CheckVkResult(vkDeviceWaitIdle(mDevice), "Failed to idle device!");
        }

        void VulkanDevice::SubmitQueue(const CommandQueueSubmitInfo& info) {
            VulkanCommandQueue* vkQueue = static_cast<VulkanCommandQueue*>(info.queue);
            auto glock = AcquireQueueAccess();
            auto qlock = vkQueue->AcquireAccess(); // vulkan requires queue submissions to be synchronised!

            const u64 localQueueCpuTimelineValue = vkQueue->IncGetCpuTimelineValue();

            VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };

            eastl::hash_set<VulkanSwapChain*> pendingSwapChainPresents;
            eastl::vector<VkCommandBufferSubmitInfo> vkCommandBuffers = {};
            for (ICommandBuffer* commandBuffer : info.commands) {
                VulkanCommandBuffer* cmb = static_cast<VulkanCommandBuffer*>(commandBuffer);
                vkCommandBuffers.push_back({ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .commandBuffer = cmb->GetVkCommandBuffer() });
                pendingSwapChainPresents.insert(
                    cmb->GetSwapchainReferences().begin(), cmb->GetSwapchainReferences().end());
            }
            submitInfo.pCommandBufferInfos = vkCommandBuffers.data();
            submitInfo.commandBufferInfoCount = vkCommandBuffers.size();

            eastl::vector<VkSemaphoreSubmitInfo> waitSemaphores = {};
            eastl::vector<VkSemaphoreSubmitInfo> signalSemaphores = {};

            { // Queue specific timeline
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = vkQueue->GetGpuTimeline()->GetVkSemaphore();
                semaphoreSubmit.value = localQueueCpuTimelineValue;
                semaphoreSubmit.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                signalSemaphores.push_back(semaphoreSubmit);
            }
            // track pending submits
            mQueuePendingSubmits.PushBack({
                .localCpuTimelineValue = localQueueCpuTimelineValue,
                .queue = vkQueue,
            });

            for (auto [fence, index] : info.signalFences) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = static_cast<VulkanFence*>(fence)->GetVkSemaphore();
                semaphoreSubmit.value = index;
                semaphoreSubmit.stageMask = /*TODO: is this efficient? -> */ VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                signalSemaphores.push_back(semaphoreSubmit);
            }
            for (auto [semaphore, stage] : info.signalSemaphores) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = ToVkPipelineStageFlags(stage);
                signalSemaphores.push_back(semaphoreSubmit);
            }

            eastl::vector<VkPipelineStageFlags2> waitStageMasks = {};
            waitStageMasks.reserve(info.waitSemaphores.size() + pendingSwapChainPresents.size());

            for (auto [semaphore, stage] : info.waitSemaphores) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                semaphoreSubmit.semaphore = eastl::bit_cast<VulkanSemaphore*>(semaphore)->GetVkSemaphore();
                semaphoreSubmit.stageMask = ToVkPipelineStageFlags(stage);
                waitSemaphores.push_back(semaphoreSubmit);
            }
            for (auto* vkswapch : pendingSwapChainPresents) {
                VkSemaphoreSubmitInfo semaphoreSubmit{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
                // TODO: Since we know when these swapchains are referenced, we can infer the stage flags
                semaphoreSubmit.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

                semaphoreSubmit.semaphore = vkswapch->GetCurrentRenderFinishSemaphore();
                signalSemaphores.push_back(semaphoreSubmit);
                semaphoreSubmit.semaphore = vkswapch->GetCurrentImageAcquireSemaphore();
                waitSemaphores.push_back(semaphoreSubmit);
            }
            // TODO for Lukas, add timeline semaphores here?

            submitInfo.waitSemaphoreInfoCount = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
            submitInfo.signalSemaphoreInfoCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();

            CheckVkResult(vkQueueSubmit2(vkQueue->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE), "Failed to submit command queue!");

            for (ICommandBuffer* commandBuffer : info.commands) {
                auto* vkcmd = static_cast<VulkanCommandBuffer*>(commandBuffer);
                mMainQueueCommandListZombies.EmplaceBack(
                    localQueueCpuTimelineValue,
                    CommandListZombie{
                        .vkCmdBuffer = vkcmd->GetVkCommandBuffer(),
                        .vkCmdPool = vkcmd->GetVkCommandPool(),
                        .queue = vkQueue,
                    });
                vkQueue->DecrementOpenCommands();
                delete vkcmd;
            }
        }

        void VulkanDevice::PresentQueue(const CommandQueuePresentInfo& info) {
            VulkanCommandQueue* vkQueue = static_cast<VulkanCommandQueue*>(info.queue);
            if (info.swapChains.empty()) {
                return;
            }
            auto lock = vkQueue->AcquireAccess();

            eastl::vector<VkResult> result(info.swapChains.size());
            eastl::vector<VkSemaphore> waitSemaphores = {};
            eastl::vector<u32> swapchainIndices = {};
            eastl::vector<VkSwapchainKHR> swapchains = {};
            eastl::for_each(info.swapChains.begin(), info.swapChains.end(), [&](ISwapChain* swapchain) {
                auto* vkSwapchain = static_cast<VulkanSwapChain*>(swapchain);
                waitSemaphores.push_back(vkSwapchain->GetCurrentRenderFinishSemaphore());
                swapchainIndices.push_back(vkSwapchain->GetCurrentImageIndex());
                swapchains.push_back(vkSwapchain->GetVkSwapChain());
            });

            VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            presentInfo.pImageIndices = swapchainIndices.data();
            presentInfo.pSwapchains = swapchains.data();
            presentInfo.swapchainCount = swapchains.size();
            presentInfo.pWaitSemaphores = waitSemaphores.data();
            presentInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
            presentInfo.pResults = result.data();
            vkQueuePresentKHR(vkQueue->GetVkQueue(), &presentInfo);
        }

        bool VulkanDevice::CollectGarbage() {
            std::unique_lock lck(mGlobalQueueMutex); // lock any command queue access
            // check if any command lists are open!! If they are, return false because it's not safe to collect right now!
            for (ICommandQueue* commandQueue : mCommandQueues) {
                if (static_cast<VulkanCommandQueue*>(commandQueue)->HasOpenCommands()) {
                    return false;
                }
            }

            for (int i = 0; i < mMainQueueCommandListZombies.Size(); ++i) {
                auto [deleteTimeline, object] = mMainQueueCommandListZombies.At(i);
                if (object.queue->GetGpuTimeline()->Value() >= deleteTimeline) {
                    vkResetCommandPool(mDevice, object.vkCmdPool, {});
                    object.queue->GetCommandBufferPool()->PutBack({ object.vkCmdPool, object.vkCmdBuffer });
                    mMainQueueCommandListZombies.Erase(i);
                    --i;
                }
            }

            eastl::vector<eastl::pair<VulkanCommandQueue*, u64>> oldestQueueTimelines = {};
            oldestQueueTimelines.reserve(mCommandQueues.size());
            for (ICommandQueue* commandQueue : mCommandQueues) {
                oldestQueueTimelines.emplace_back(static_cast<VulkanCommandQueue*>(commandQueue), UINT64_MAX);
            }
            {
                std::lock_guard lck(mQueuePendingSubmits.GetLock());
                auto& submitVec = mQueuePendingSubmits.UnderlyingVector();
                for (int i = 0; i < submitVec.size(); ++i) {
                    auto [queueTimeline, queue] = submitVec.at(i);
                    u64 completedValue = queue->GetGpuTimeline()->Value();
                    if (completedValue >= queueTimeline) {
                        submitVec.erase(submitVec.begin() + i);
                        --i;
                    } else {
                        for (auto& [oldestTimelineQueue, oldestVal] : oldestQueueTimelines) {
                            if (queue != oldestTimelineQueue)
                                continue;
                            oldestVal = eastl::min(oldestVal, completedValue);
                        }
                    }
                }
            }

            for (int i = 0; i < mResourceZombies.Size(); ++i) {
                auto [deleteTimelineSnapshot, zombie] = mResourceZombies.At(i);

                bool bReady = true;
                // Check across all queues to see what is pending.
                // Queues that are not participating will just have a value of UINT64_MAX and naturally skip the check
                for (int j = 0; j < oldestQueueTimelines.size(); ++j) {
                    auto [queue, completedTimeline] = oldestQueueTimelines[j];
                    // resources should NEVER catch up to the main cpu timeline,
                    // otherwise they were scheduled for destruction after a final queue submission (BAD)
                    // So a "less than equal" condition is necessary to determine if this is not ready.
                    if (completedTimeline <= deleteTimelineSnapshot[j]) {
                        bReady = false;
                        break;
                    }
                }
                if (bReady) {
                    zombie.deleter(this, zombie.resource); // destroy
                    mResourceZombies.Erase(i);
                    --i;
                }
            }
            return true;
        }

        const DeviceInfo& VulkanDevice::Info() const {
            return mInfo;
        }
        const DevicePropertiesInfo& VulkanDevice::Properties() const {
            return mProperties;
        }

        const DeviceFeaturesInfo& VulkanDevice::Features() const {
            return mFeatures;
        }

        DeviceStatusInfo VulkanDevice::Status() const {
            return {};
        }

        u32 VulkanDevice::GetActiveShaderModel() const {
            return mActiveShaderModel;
        }

        void VulkanDevice::SetShaderModel(u32 shaderModel) {
            ASSERT(shaderModel <= mFeatures.maxSupportedShaderModel, "Shader model used is unsupported!");
            mActiveShaderModel = shaderModel;
        }

        Image VulkanDevice::NewSwapChainImage(VulkanSwapChain* owner, VkImage swapchainImage, VkFormat format, u32 index, ImageUsageFlags usage, const ImageInfo& imageInfo) {
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
            ret.swapchainImageIndex = static_cast<i32>(index);
            ret.ownedSwapchain = owner;
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
        auto VulkanDevice::Slot(MemoryBlock id) -> ImplVmaVirtualBlockSlot& { return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(Buffer id) -> ImplBufferSlot& { return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(Image id) -> ImplImageSlot& { return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(ShaderResourceId id) -> ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(UnorderedAccessId id) -> ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(SamplerId id) -> ImplSamplerSlot& { return mResourceTable.mSamplerSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(BlasId id) -> ImplBlasSlot& { return mResourceTable.mBlasSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(TlasId id) -> ImplTlasSlot& { return mResourceTable.mTlasSlots.DereferenceId(id); }

        auto VulkanDevice::Slot(MemoryBlock id) const -> const ImplVmaVirtualBlockSlot& { return mResourceTable.mVirtualBlockSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(Buffer id) const -> const ImplBufferSlot& { return mResourceTable.mBufferSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(Image id) const -> const ImplImageSlot& { return mResourceTable.mImageSlots.DereferenceId(eastl::bit_cast<GpuResourceId>(id)); }
        auto VulkanDevice::Slot(ShaderResourceId id) const -> const ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(UnorderedAccessId id) const -> const ImplResourceViewSlot& { return mResourceTable.mResourceViewSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(SamplerId id) const -> const ImplSamplerSlot& { return mResourceTable.mSamplerSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(BlasId id) const -> const ImplBlasSlot& { return mResourceTable.mBlasSlots.DereferenceId(id); }
        auto VulkanDevice::Slot(TlasId id) const -> const ImplTlasSlot& { return mResourceTable.mTlasSlots.DereferenceId(id); }

        eastl::vector<u64> VulkanDevice::SnapshotQueueTimelineValues() const {
            eastl::vector<u64> values = {};
            values.reserve(mCommandQueues.size());
            for (ICommandQueue* q : mCommandQueues) {
                auto* vkQueue = static_cast<VulkanCommandQueue*>(q);
                values.emplace_back(vkQueue->GetCpuTimelineValue());
            }
            return values;
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios