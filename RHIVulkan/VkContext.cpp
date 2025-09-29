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

#include "VkContext.hpp"
#include "Api/Device.hpp"
#include <EASTL/hash_map.h>

#include <PyroRHI/Api/Limits.hpp>
#include <PyroRHI/Shader/IShaderFeatureSet.hpp>

#include <EASTL/vector.h>
#include <PyroCommon/Logger.hpp>
#include <iostream>

namespace PyroshockStudios::RHIVulkan {

    static bool IsDeviceSuitable(VkPhysicalDevice device, eastl::vector<eastl::string>& whatWasntFound) {
        whatWasntFound.clear();

        // --- Query all features first ---
        VkPhysicalDeviceBufferDeviceAddressFeatures bufferAddressFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, &bufferAddressFeatures };
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES, &descriptorIndexingFeatures };
        VkPhysicalDeviceSynchronization2Features sync2Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, &dynamicRenderingFeatures };
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES, &sync2Features };
        VkPhysicalDeviceScalarBlockLayoutFeatures scalarBlockLayoutFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES, &timelineSemaphoreFeatures };
        VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParamsFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES, &scalarBlockLayoutFeatures };

        VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &shaderDrawParamsFeatures };
        vkGetPhysicalDeviceFeatures2(device, &features);

        auto AddFeatureIfMissing = [&](bool feature, const char* name) {
            if (!feature)
                whatWasntFound.emplace_back(eastl::string("Missing feature: ") + name);
        };

        // Vulkan 1.0 core features
        AddFeatureIfMissing(features.features.imageCubeArray, "imageCubeArray");
        AddFeatureIfMissing(features.features.independentBlend, "independentBlend");
        AddFeatureIfMissing(features.features.geometryShader, "geometryShader");
        AddFeatureIfMissing(features.features.tessellationShader, "tessellationShader");
        AddFeatureIfMissing(features.features.sampleRateShading, "sampleRateShading");
        AddFeatureIfMissing(features.features.multiDrawIndirect, "multiDrawIndirect");
        AddFeatureIfMissing(features.features.drawIndirectFirstInstance, "drawIndirectFirstInstance");
        AddFeatureIfMissing(features.features.depthClamp, "depthClamp");
        AddFeatureIfMissing(features.features.fillModeNonSolid, "fillModeNonSolid");
        AddFeatureIfMissing(features.features.wideLines, "wideLines");
        AddFeatureIfMissing(features.features.samplerAnisotropy, "samplerAnisotropy");
        AddFeatureIfMissing(features.features.textureCompressionBC, "textureCompressionBC");
        AddFeatureIfMissing(features.features.fragmentStoresAndAtomics, "fragmentStoresAndAtomics");
        AddFeatureIfMissing(features.features.shaderImageGatherExtended, "shaderImageGatherExtended");
        AddFeatureIfMissing(features.features.shaderStorageImageMultisample, "shaderStorageImageMultisample");
        // AddFeatureIfMissing(features.features.shaderStorageImageReadWithoutFormat, "shaderStorageImageReadWithoutFormat");
        // AddFeatureIfMissing(features.features.shaderStorageImageWriteWithoutFormat, "shaderStorageImageWriteWithoutFormat");
        AddFeatureIfMissing(features.features.shaderClipDistance, "shaderClipDistance");

        // Vulkan 1.1 / 1.2 / 1.3 features
        AddFeatureIfMissing(shaderDrawParamsFeatures.shaderDrawParameters, "Vulkan 1.1 Shader Draw Parameters");
        AddFeatureIfMissing(timelineSemaphoreFeatures.timelineSemaphore, "Vulkan 1.1 Timeline Semaphore");
        AddFeatureIfMissing(sync2Features.synchronization2, "Vulkan 1.2 Synchronization2");

        AddFeatureIfMissing(descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing, "Descriptor Indexing: shaderSampledImageArrayNonUniformIndexing");
        AddFeatureIfMissing(descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing, "Descriptor Indexing: shaderStorageBufferArrayNonUniformIndexing");
        AddFeatureIfMissing(descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing, "Descriptor Indexing: shaderStorageImageArrayNonUniformIndexing");
        AddFeatureIfMissing(descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind, "Descriptor Indexing: descriptorBindingSampledImageUpdateAfterBind");
        AddFeatureIfMissing(descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind, "Descriptor Indexing: descriptorBindingStorageImageUpdateAfterBind");
        AddFeatureIfMissing(descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind, "Descriptor Indexing: descriptorBindingStorageBufferUpdateAfterBind");
        AddFeatureIfMissing(descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending, "Descriptor Indexing: descriptorBindingUpdateUnusedWhilePending");
        AddFeatureIfMissing(descriptorIndexingFeatures.descriptorBindingPartiallyBound, "Descriptor Indexing: descriptorBindingPartiallyBound");
        AddFeatureIfMissing(descriptorIndexingFeatures.runtimeDescriptorArray, "Descriptor Indexing: runtimeDescriptorArray");

        AddFeatureIfMissing(dynamicRenderingFeatures.dynamicRendering, "Vulkan 1.3 Dynamic Rendering");

        // Optional but recommended
        AddFeatureIfMissing(bufferAddressFeatures.bufferDeviceAddress, "Vulkan 1.3 Buffer Device Address");
        AddFeatureIfMissing(features.features.shaderInt64, "Vulkan 1.3 shaderInt64");
        AddFeatureIfMissing(scalarBlockLayoutFeatures.scalarBlockLayout, "Vulkan 1.3 Scalar Block Layout");

        // --- Check properties ---
        VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR };
        VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES, &pushDescriptorProps };

        VkPhysicalDeviceProperties2 properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &descriptorIndexingProps };
        vkGetPhysicalDeviceProperties2(device, &properties);

        auto AddPropertyIfTooLow = [&](uint32_t value, uint32_t minRequired, const char* name) {
            if (value < minRequired) {
                whatWasntFound.emplace_back(eastl::string("Property too low: ") + name +
                                            " (found " + eastl::to_string(value) +
                                            ", required " + eastl::to_string(minRequired) + ")");
            }
        };

        AddPropertyIfTooLow(properties.properties.limits.maxColorAttachments, 8, "maxColorAttachments");
        AddPropertyIfTooLow(properties.properties.limits.maxPushConstantsSize, Limits::MAX_PUSH_CONSTANT_SIZE, "maxPushConstantsSize");
        AddPropertyIfTooLow(properties.properties.limits.maxUniformBufferRange, Limits::MAX_UNIFORM_BUFFER_SIZE, "maxUniformBufferRange");
        AddPropertyIfTooLow(properties.properties.limits.maxPerStageDescriptorUniformBuffers, Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS, "maxPerStageDescriptorUniformBuffers");
        AddPropertyIfTooLow(descriptorIndexingProps.maxDescriptorSetUpdateAfterBindSampledImages, MAX_VK_BINDLESS_IMAGES, "maxDescriptorSetUpdateAfterBindSampledImages");
        AddPropertyIfTooLow(descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindSampledImages, MAX_VK_BINDLESS_IMAGES, "maxPerStageDescriptorUpdateAfterBindSampledImages");
        AddPropertyIfTooLow(descriptorIndexingProps.maxDescriptorSetUpdateAfterBindStorageImages, MAX_VK_BINDLESS_IMAGES, "maxDescriptorSetUpdateAfterBindStorageImages");
        AddPropertyIfTooLow(descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindStorageImages, MAX_VK_BINDLESS_IMAGES, "maxPerStageDescriptorUpdateAfterBindStorageImages");
        AddPropertyIfTooLow(descriptorIndexingProps.maxDescriptorSetUpdateAfterBindStorageBuffers, MAX_VK_BINDLESS_BUFFERS, "maxDescriptorSetUpdateAfterBindStorageBuffers");
        AddPropertyIfTooLow(descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindStorageBuffers, MAX_VK_BINDLESS_BUFFERS, "maxPerStageDescriptorUpdateAfterBindStorageBuffers");
        AddPropertyIfTooLow(descriptorIndexingProps.maxDescriptorSetUpdateAfterBindSamplers, MAX_VK_BINDLESS_SAMPLERS, "maxDescriptorSetUpdateAfterBindSamplers");
        AddPropertyIfTooLow(descriptorIndexingProps.maxPerStageDescriptorUpdateAfterBindSamplers, MAX_VK_BINDLESS_SAMPLERS, "maxPerStageDescriptorUpdateAfterBindSamplers");
        AddPropertyIfTooLow(pushDescriptorProps.maxPushDescriptors, 32, "maxPushDescriptors");

        return whatWasntFound.empty();
    }


    void* VulkanContext::OnAllocate(void* userData, usize size, usize alignment, VkSystemAllocationScope scope) {
        return malloc(size);
    }
    void* VulkanContext::OnReallocate(void* userData, void* original, usize size, usize alignment, VkSystemAllocationScope scope) {
        return realloc(original, size);
    }
    void VulkanContext::OnFree(void* userData, void* mem) {
        free(mem);
    }
    void VulkanContext::OnAllocNotify(void* userData, usize size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
        VulkanContext* context = reinterpret_cast<VulkanContext*>(userData);
        ++context->mNumAllocations;
        context->mNumAllocatedBytes += size;
    }
    void VulkanContext::OnFreeNotify(void* userData, usize size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
        VulkanContext* context = reinterpret_cast<VulkanContext*>(userData);
        --context->mNumAllocations;
        context->mNumAllocatedBytes -= size;
    }

    VulkanContext::VulkanContext(const VulkanContextArgs& args, const ILogStream* logSink) : mPreferredDeviceIndex(args.preferredPhysicalDevice) {
        VulkanContext::InjectLogger(logSink);
        volkInitialize();

        eastl::vector<char const*> explicitExtensions = {};
        if (args.bEnableValidation) {
            explicitExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        eastl::vector<char const*> implicitExtensions = {};
        implicitExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        implicitExtensions.push_back("VK_KHR_win32_surface");
        implicitExtensions.push_back("VK_KHR_xlib_surface");
        implicitExtensions.push_back("VK_KHR_wayland_surface");

        eastl::vector<VkExtensionProperties> instance_extensions = {};
        uint32_t instance_extension_count = {};
        auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        CheckVkResult(result);

        instance_extensions.resize(instance_extension_count);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
        CheckVkResult(result);

        eastl::vector<char const*> enabledExtensions = {};
        for (auto const* req_ext : explicitExtensions) {
            bool found = false;
            for (auto& instance_extension : instance_extensions) {
                if (strcmp(req_ext, instance_extension.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("bruh");
            }
            enabledExtensions.push_back(req_ext);
        }

        for (const char* ext : implicitExtensions) {
            bool found = false;
            for (auto& instance_extension : instance_extensions) {
                if (strcmp(ext, instance_extension.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (found) {
                enabledExtensions.push_back(ext);
            }
        }
        eastl::string enabledextensionStr = "";
        for (auto& extenstion : enabledExtensions) {
            enabledextensionStr += "\n    ";
            enabledextensionStr += extenstion;
        }
        Logger::Info(gVulkanSink, "Enabled Extensions:{}", enabledextensionStr);
        const VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = args.appName,
            .applicationVersion = args.appVersion,
            .pEngineName = args.engineName,
            .engineVersion = args.engineVersion,
            .apiVersion = VK_API_VERSION_1_3,
        };

        const VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0u,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data(),
        };

        CreateAllocationCallbacks();

        result = vkCreateInstance(&instanceCreateInfo, &mAllocator, &mInstance);
        CheckVkResult(result);
        volkLoadInstance(mInstance);
    }

    VulkanContext::~VulkanContext() {
        for (VulkanDevice* device : mCreatedDevices) {
            delete device;
        }
        vkDestroyInstance(mInstance, &mAllocator);
        if (mNumAllocations > 0) {
            Logger::Error(gVulkanSink, "Leaked " + eastl::to_string(mNumAllocatedBytes) + " bytes! (" + eastl::to_string(mNumAllocations) + " leaked allocations)");
        }
    }


    IDevice* VulkanContext::CreateDevice() {
        u32 deviceCount = 0;
        auto result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
        CheckVkResult(result);

        eastl::vector<VkPhysicalDevice> vkPhysicalDevices = {};
        vkPhysicalDevices.resize(deviceCount);
        result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, vkPhysicalDevices.data());
        CheckVkResult(result);

        VkPhysicalDevice vkPhysicalDevice = {};
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties = {};

        eastl::hash_set<VkPhysicalDevice> unsuitableDevices = {};
        eastl::hash_map<VkPhysicalDevice, eastl::vector<eastl::string>> unsuitableDeviceInfos = {};
        if (mPreferredDeviceIndex == -1) { // automatically pick most suitable device
            for (const auto& device : vkPhysicalDevices) {
                vkGetPhysicalDeviceProperties(device, &vkPhysicalDeviceProperties);
                eastl::vector<eastl::string> whatNotFound;
                if (!IsDeviceSuitable(device, whatNotFound)) {
                    unsuitableDevices.emplace(device);
                    unsuitableDeviceInfos.emplace(device, eastl::move(whatNotFound));
                    continue;
                }
                Logger::Trace(gVulkanSink, "Found physical device: " + eastl::string(vkPhysicalDeviceProperties.deviceName));
                if (vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    vkPhysicalDevice = device;
                    break;
                }
            }
        } else {
            if (mPreferredDeviceIndex < vkPhysicalDevices.size() && mPreferredDeviceIndex >= 0) {
                vkPhysicalDevice = vkPhysicalDevices[mPreferredDeviceIndex];
                vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
                eastl::vector<eastl::string> whatNotFound;
                if (!IsDeviceSuitable(vkPhysicalDevice, whatNotFound)) {
                    unsuitableDevices.emplace(vkPhysicalDevice);
                    unsuitableDeviceInfos.emplace(vkPhysicalDevice, eastl::move(whatNotFound));
                    eastl::string notFoundList = {};
                    for (auto& str : whatNotFound) {
                        notFoundList += "\t";
                        notFoundList += str;
                        notFoundList += "\n";
                    }
                    Logger::Warn(gVulkanSink, "Preferred device " + eastl::string(vkPhysicalDeviceProperties.deviceName) + " is not supported! Reason: " + notFoundList);
                }
            }
        }
        if (vkPhysicalDevice == VK_NULL_HANDLE) {
            // pick fallback
            for (auto device : vkPhysicalDevices) {
                if (unsuitableDevices.contains(device)) {
                    continue;
                }
                vkPhysicalDevice = vkPhysicalDevices[0];
            }
            if (vkPhysicalDevice == VK_NULL_HANDLE) {
                Logger::Error(gVulkanSink, "Failed to pick any vulkan device!");
                for (const auto& [devc, errors] : unsuitableDeviceInfos) {
                    vkGetPhysicalDeviceProperties(devc, &vkPhysicalDeviceProperties);
                    Logger::Error( gVulkanSink, "Device " + eastl::string(vkPhysicalDeviceProperties.deviceName) + " is not suitable for the following reasons:");
                    for (const auto& msg : errors) {
                        Logger::Error(gVulkanSink, "\t" + eastl::string(msg));
                    }
                }
                Logger::Fatal(gVulkanSink, "Vulkan device creation error!");
            }
            vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
            Logger::Warn(gVulkanSink, "Selected fallback device " + eastl::string(vkPhysicalDeviceProperties.deviceName));
        }

        Logger::Info(gVulkanSink, "Using physical device: " + eastl::string(vkPhysicalDeviceProperties.deviceName));

        VulkanDevice* device = new VulkanDevice(this, vkPhysicalDevice);
        mCreatedDevices.push_back(device);
        return device;
    }

    const RHIProperties& VulkanContext::Properties() {
        const static RHIProperties set{
            .bBufferDeviceAddress = true,
            .bScalarLayout = true,
            .bDrawIndirectCount = true,
            .bUint8IndexBuffer = true,
            .viewportConvention = RHIViewportConvention::LeftHanded_OriginTopLeft,
            .bufferImageRowAlignment = 1,
        };
        return set;
    }

    IShaderFeatureSet* VulkanContext::ShaderFeatureSet() {
        struct SlangVulkan13FeatureSetStub : public IShaderFeatureSet {
            SlangVulkan13FeatureSetStub() = default;
            ShaderCompileTarget GetTarget() const override {
                return ShaderCompileTarget::Spirv;
            }
            const char* GetProfileName(ShaderStage shaderStage) const override {
                return "glsl_460";
            }
            const char* GetFileExtension() const override {
                return "spv";
            }
            const ShaderFeatureInfo& Features() const override {
                static auto features = ShaderFeatureInfo{
                    .bDescriptorIndexing = true,
                    .bBufferDeviceAddress = true,
                    .bScalarLayout = true,
                    .bDrawParameters = true,
                    .bGLSL = true,
                };
                return features;
            }
            const eastl::span<eastl::pair<const char*, const char*>>& GlobalPreprocessorDefines() const override {
                static eastl::vector<eastl::pair<const char*, const char*>> preprocesor = {
                    { "PYRO_SHADER_FLAG_RHI_VK13", "1" },
                    { "PYRO_SHADER_FLAG_ENABLE_SPECIALIZATION_CONSTANTS", "1" },
                    { "PYRO_SHADER_FLAG_ENABLE_GL_DRAW_ID", "1" },
                    { "PYRO_SHADER_FLAG_ENABLE_GL_FIRST_VERTEX", "1" },
                    { "PYRO_SHADER_FLAG_ENABLE_GL_FIRST_INSTANCE", "1" },
                };
                static auto span = eastl::span(preprocesor.data(), preprocesor.size());
                return span;
            }
        };
        // for now
        static SlangVulkan13FeatureSetStub stub{};
        return &stub;
    }

    void VulkanContext::InjectLogger(const ILogStream* stream) {
        gVulkanSink = stream;
    }

    void VulkanContext::CreateAllocationCallbacks() {
        mAllocator = VkAllocationCallbacks{
            .pUserData = reinterpret_cast<void*>(this),
            .pfnAllocation = VulkanContext::OnAllocate,
            .pfnReallocation = VulkanContext::OnReallocate,
            .pfnFree = VulkanContext::OnFree,
            .pfnInternalAllocation = VulkanContext::OnAllocNotify,
            .pfnInternalFree = VulkanContext::OnFreeNotify,
        };
    }
} // namespace PyroshockStudios::RHIVulkan