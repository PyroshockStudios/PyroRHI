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
            VulkanContext(const VulkanContextArgs& args, const ILogStream* logSink);
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

            void InjectLogger(const ILogStream* stream) override;

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