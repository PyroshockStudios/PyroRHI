#pragma once
#include "GPUResourcePool.hpp"
#include <PyroRHI/Api/RenderTarget.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;
        class VulkanRenderTarget : DeleteCopy, DeleteMove {
        public:
            VulkanRenderTarget(VulkanDevice* device, const RenderTargetInfo& info);
            ~VulkanRenderTarget();

            const RenderTargetInfo& Info() const;

            VkImageView GetVkImageView() {
                return mImageView;
            }

        private:
            RenderTargetInfo mInfo = {};
            VkImageView mImageView = {};
            VulkanDevice* mDevice;
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios