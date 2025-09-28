#pragma once
#include "Core.hpp"
#include <PyroCommon/Version.hpp>
#include <PyroRHI/Context.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;
        struct VulkanContextArgs {
            const char* appName = "";
            const char* engineName = "";
            u32 appVersion = {};
            u32 engineVersion = {};
            bool bEnableValidation = false;
            i64 preferredPhysicalDevice = -1; // -1 == pick most suitable
        };
        class VulkanContext : public RHIContext {
        public:
            VulkanContext(const VulkanContextArgs& args);
            ~VulkanContext();

            IDevice* CreateDevice() override;
            const RHIProperties& Properties() override;
            IShaderFeatureSet* ShaderFeatureSet() override;

            const VkAllocationCallbacks* GetVkAllocator() {
                return &mAllocator;
            }
            VkInstance GetVkInstance() {
                return mInstance;
            }

        private:
            void CreateAllocationCallbacks();

            static void* OnAllocate(void* userData, usize size, usize alignment, VkSystemAllocationScope scope);
            static void* OnReallocate(void* userData, void* original, usize size, usize alignment, VkSystemAllocationScope scope);
            static void OnFree(void* userData, void* mem);
            static void OnAllocNotify(void* userData, usize size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope);
            static void OnFreeNotify(void* userData, usize size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope);

            eastl::vector<VulkanDevice*> mCreatedDevices = {};

            VkInstance mInstance = {};
            VkAllocationCallbacks mAllocator = {};
            i64 mPreferredDeviceIndex = {};

            usize mNumAllocations = {};
            usize mNumAllocatedBytes = {};
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios