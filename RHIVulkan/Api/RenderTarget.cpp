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
            CheckVkResult(result, "Failed to create render target image view!");

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                    .objectHandle = eastl::bit_cast<uint64_t>(mImageView),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameInfo);
            }
        }
        VulkanRenderTarget::~VulkanRenderTarget() {
            vkDestroyImageView(mDevice->GetVkDevice(), mImageView, mDevice->Context()->GetVkAllocator());
        }
        const RenderTargetInfo& VulkanRenderTarget::Info() const {
            return mInfo;
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios
