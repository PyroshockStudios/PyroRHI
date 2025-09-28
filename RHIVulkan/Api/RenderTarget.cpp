#include "RenderTarget.hpp"
#include "Device.hpp"

#include <RHIVulkan/VkContext.hpp>

#include <libassert/assert.hpp>
namespace PyroshockStudios {
    namespace RHIVulkan {
        VulkanRenderTarget::VulkanRenderTarget(VulkanDevice* device, const RenderTargetInfo& info)
            : mDevice(device), mInfo(info) {
            ImplImageSlot& imageSlot = mDevice->Slot(info.image);

            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = ToVkFormat(imageSlot.info.format);
            VkImageSubresourceRange subresourceRange{};
            if (info.flags & RenderTargetFlagBits::COLOR_TARGET) {
                subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
                ASSERT(!RHIUtil::FormatIsDepthStencil(imageSlot.info.format), "For a render target to have the COLOR aspect, it must not be created from a depth-stencil format!");
            }
            if (info.flags & RenderTargetFlagBits::DEPTH_TARGET) {
                subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
                ASSERT(RHIUtil::FormatHasDepth(imageSlot.info.format), "For a render target to have the DEPTH aspect, it must be created from an image with a depth format!");
            }
            if (info.flags & RenderTargetFlagBits::STENCIL_TARGET) {
                subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                ASSERT(RHIUtil::FormatHasStencil(imageSlot.info.format), "For a render target to have the STENCIL aspect, it must be created from an image with a stencil format!");
            }

            subresourceRange.baseMipLevel = info.slice.mipLevel;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = info.slice.arrayLayer;
            subresourceRange.layerCount = 1;
            imageViewCreateInfo.subresourceRange = subresourceRange;
            imageViewCreateInfo.image = imageSlot.vkImage;
            VkResult result = vkCreateImageView(mDevice->GetVkDevice(), &imageViewCreateInfo, mDevice->Context()->GetVkAllocator(), &mImageView);
            CheckVkResult(result);
        }
        VulkanRenderTarget::~VulkanRenderTarget() {
            vkDestroyImageView(mDevice->GetVkDevice(), mImageView, mDevice->Context()->GetVkAllocator());
        }
        const RenderTargetInfo& VulkanRenderTarget::Info() const {
            return mInfo;
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios
