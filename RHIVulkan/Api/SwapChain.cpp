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

#include "SwapChain.hpp"

#include <EASTL/numeric_limits.h>

#include "Device.hpp"

#include <PyroCommon/Logger.hpp>
#include <RHIVulkan/VkContext.hpp>

#ifdef PYRO_PLATFORM_WINDOWS
#include <Windows.h>
#elif PYRO_PLATFORM_LINUX
#include <X11/Xlib.h>
#endif
#include <libassert/assert.hpp>

namespace PyroshockStudios::RHIVulkan {
    PYRO_FORCEINLINE static constexpr VkPresentModeKHR ToVkPresentMode(SwapChainPresentMode type) { return static_cast<VkPresentModeKHR>(type); }

    VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, const SwapChainInfo& info) : mInfo(info), mDevice(device) {
        CreateSurface();
        mSupportInfo = device->GetSwapChainSupport(mSurface);
        VkSurfaceFormatKHR finalFormat{};
        bool bFoundFormat = false;
        for (const auto& availableFormat : mSupportInfo.formats) {
            if (bFoundFormat)
                break;
            switch (info.format) {
            case SwapChainFormat::Unorm8BitLDR:
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    finalFormat = availableFormat;
                    mFormat = Format::BGRA8Unorm;
                    mColorSpace = ColorSpace::SrgbNonlinear;
                    bFoundFormat = true;
                } else if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    mFormat = Format::RGBA8Unorm;
                    mColorSpace = ColorSpace::SrgbNonlinear;
                    finalFormat = availableFormat;
                    bFoundFormat = true;
                }
                break;
            case SwapChainFormat::Srgb8BitLDR:
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    mFormat = Format::BGRA8Srgb;
                    mColorSpace = ColorSpace::SrgbNonlinear;
                    finalFormat = availableFormat;
                    bFoundFormat = true;
                } else if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    mFormat = Format::RGBA8Srgb;
                    mColorSpace = ColorSpace::SrgbNonlinear;
                    finalFormat = availableFormat;
                    bFoundFormat = true;
                }
                break;
            case SwapChainFormat::Unorm10BitLDR:
                if (availableFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    mFormat = Format::A2RGB10Unorm;
                    mColorSpace = ColorSpace::SrgbNonlinear;
                    finalFormat = availableFormat;
                    bFoundFormat = true;
                }
                break;
            case SwapChainFormat::Float16BitHDR: // idfk, just dont use hdr displays bro
                if (availableFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
                    mFormat = Format::RGBA16Sfloat;
                    mColorSpace = ColorSpace::Hdr10St2084;
                    finalFormat = availableFormat;
                    bFoundFormat = true;
                }
                break;
            }
        }
        if (finalFormat.format == VK_FORMAT_UNDEFINED) {
            Logger::Fatal(gVulkanSink, "SwapChain format is not available!");
        }
        TrySetPresentMode(info.presentMode);

        CreateSwapChain(VK_NULL_HANDLE);
        CreateSemaphores();
    }
    VulkanSwapChain::~VulkanSwapChain() {
        for (i32 i = 0; i < mInfo.bufferCount; ++i) {
            mDevice->DestroyImage(mWrappedImages[i]);
            vkDestroySemaphore(mDevice->GetVkDevice(), mImageAcquireSemaphores[i], mDevice->Context()->GetVkAllocator());
        }
        vkDestroySwapchainKHR(mDevice->GetVkDevice(), mSwapChain, mDevice->Context()->GetVkAllocator());
        vkDestroySurfaceKHR(mDevice->Context()->GetVkInstance(), mSurface, mDevice->Context()->GetVkAllocator());
    }

    Image VulkanSwapChain::GetBackBuffer(i32 imageIndex) {
        if (imageIndex >= mWrappedImages.size())
            return {};
        return mWrappedImages[imageIndex];
    }
    i32 VulkanSwapChain::AcquireNextImage() {
        mImageAcquireIndex = (mImageAcquireIndex + 1) % mInfo.bufferCount;
        VkResult result = vkAcquireNextImageKHR(mDevice->GetVkDevice(),
            mSwapChain,
            eastl::numeric_limits<u64>::max(),
            mImageAcquireSemaphores[mImageAcquireIndex],
            VK_NULL_HANDLE,
            &mImageIndex);
        return result == VK_SUCCESS ? mImageIndex : PYRO_SWAPCHAIN_ACQUIRE_FAIL;
    }

    void VulkanSwapChain::Resize() {
        mSupportInfo = mDevice->GetSwapChainSupport(mSurface);
        mInfo.extent = { mSupportInfo.capabilities.currentExtent.width, mSupportInfo.capabilities.currentExtent.height };
        mImageIndex = 0;
        for (Image img : mWrappedImages) {
            mDevice->DestroyImage(img);
        }
        mWrappedImages.clear();
        VkSwapchainKHR oldSwapChain = mSwapChain;
        CreateSwapChain(oldSwapChain);
        vkDestroySwapchainKHR(mDevice->GetVkDevice(), oldSwapChain, mDevice->Context()->GetVkAllocator());
    }
    void VulkanSwapChain::SetPresentMode(SwapChainPresentMode presentMode) {
        TrySetPresentMode(presentMode);
    }
    const SwapChainInfo& VulkanSwapChain::Info() const {
        return mInfo;
    }
    Extent2D VulkanSwapChain::GetSurfaceExtent() const {
        return mInfo.extent;
    }
    Format VulkanSwapChain::GetFormat() const {
        return mFormat;
    }
    ColorSpace VulkanSwapChain::GetColorSpace() const {
        ASSERT(false, "TODO");
        return mColorSpace;
    }
    void VulkanSwapChain::CreateSurface() {
        VkResult result = VK_ERROR_UNKNOWN;
        if (mInfo.nativeWindow) {
#ifdef PYRO_PLATFORM_WINDOWS
            VkWin32SurfaceCreateInfoKHR createInfo{
                .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .hinstance = reinterpret_cast<HINSTANCE>(mInfo.nativeInstance),
                .hwnd = reinterpret_cast<HWND>(mInfo.nativeWindow)
            };
            auto func = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(mDevice->Context()->GetVkInstance(), "vkCreateWin32SurfaceKHR"));
            result = func(mDevice->Context()->GetVkInstance(), &createInfo, mDevice->Context()->GetVkAllocator(), &mSurface);
#elif PYRO_PLATFORM_LINUX
            VkXlibSurfaceCreateInfoKHR createInfo{
                .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                .dpy = reinterpret_cast<Display*>(mInfo.nativeInstance),
                .window = reinterpret_cast<Window>(mInfo.nativeWindow)
            };
            auto func = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(vkGetInstanceProcAddr(mDevice->Context()->GetVkInstance(), "vkCreateXlibSurfaceKHR"));
            result = func(mDevice->Context()->GetVkInstance(), &createInfo, mDevice->Context()->GetVkAllocator(), &mSurface);
#elif PYRO_PLATFORM_MACOS
            VkMacOSSurfaceCreateInfoMVK createInfo{
                .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
                .pView = reinterpret_cast<void*>(mInfo.nativeWindow)
            };
            auto func = reinterpret_cast<PFN_vkCreateMacOSSurfaceMVK>(vkGetInstanceProcAddr(mDevice->Context()->GetVkInstance(), "vkCreateMacOSSurfaceMVK"));
            result = func(mDevice->Context()->GetVkInstance(), &createInfo, mDevice->Context()->GetVkAllocator(), &mSurface);
#else
#error VulkanWindowSurface Not supported!
#endif
        } else {
            ASSERT(mDevice->Features().bHeadlessSwapChainWindow, "Cannot create a VkSurface without VK_EXT_headless_surface support!");
            VkHeadlessSurfaceCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT };
            result = vkCreateHeadlessSurfaceEXT(mDevice->Context()->GetVkInstance(), &createInfo, mDevice->Context()->GetVkAllocator(), &mSurface);
        }
        CheckVkResult(result);
    }
    void VulkanSwapChain::CreateSwapChain(VkSwapchainKHR oldSwapChain) {
        mInfo.bufferCount = eastl::clamp(mInfo.bufferCount, mSupportInfo.capabilities.minImageCount, mSupportInfo.capabilities.maxImageCount);
        mInfo.extent.width = eastl::clamp(mInfo.extent.width, mSupportInfo.capabilities.minImageExtent.width, mSupportInfo.capabilities.maxImageExtent.width);
        mInfo.extent.height = eastl::clamp(mInfo.extent.height, mSupportInfo.capabilities.minImageExtent.height, mSupportInfo.capabilities.maxImageExtent.height);

        VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mSurface;
        createInfo.minImageCount = mInfo.bufferCount;
        createInfo.imageExtent = {
            .width = mInfo.extent.width,
            .height = mInfo.extent.height,
        };
        createInfo.imageFormat = ToVkFormat(mFormat);
        createInfo.imageColorSpace = ToVkColorSpace(mColorSpace);
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = ToVkImageUsageFlags(mInfo.imageUsage, mFormat);

        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = mSupportInfo.capabilities.currentTransform;
        switch (mInfo.alphaMode) {
        case SwapChainAlphaMode::None:
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            break;
        case SwapChainAlphaMode::Premultiplied:
            if (mSupportInfo.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
            } else {
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                mInfo.alphaMode = SwapChainAlphaMode::None;
            }
            break;
        case SwapChainAlphaMode::Straight:
            if (mSupportInfo.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
            } else {
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                mInfo.alphaMode = SwapChainAlphaMode::None;
            }
            break;
        default:
            ASSERT(false, "Invalid alpha mode");
        }
        createInfo.presentMode = ToVkPresentMode(mPresentMode);
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain;

        VkResult result = vkCreateSwapchainKHR(mDevice->GetVkDevice(), &createInfo, mDevice->Context()->GetVkAllocator(), &mSwapChain);
        CheckVkResult(result);

        u32 imageCount = 0;
        vkGetSwapchainImagesKHR(mDevice->GetVkDevice(), mSwapChain, &imageCount, nullptr);
        mInfo.bufferCount = imageCount;
        mSwapImages.resize(static_cast<usize>(imageCount));
        mWrappedImages.resize(static_cast<usize>(imageCount));
        vkGetSwapchainImagesKHR(mDevice->GetVkDevice(), mSwapChain, &imageCount, mSwapImages.data());
        for (u32 i = 0; i < mSwapImages.size(); ++i) {
            if (vkSetDebugUtilsObjectNameEXT) {
                eastl::string name = mInfo.name + " (Vk Swap Image #" + eastl::to_string(i) + ")";
                const VkDebugUtilsObjectNameInfoEXT nameInfoo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_IMAGE,
                    .objectHandle = eastl::bit_cast<uint64_t>(mSwapImages[i]),
                    .pObjectName = name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameInfoo);
            }

            ImageUsageFlags usage = ImageUsageFlagBits::RENDER_TARGET | ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::TRANSFER_SRC;
            ImageInfo const image_info = {
                .format = mFormat,
                .size = { mInfo.extent.width, mInfo.extent.height, 1 },
                .usage = usage,
                .name = mInfo.name + " Image #" + eastl::to_string(i),
            };

            mWrappedImages[i] = mDevice->NewSwapChainImage(mSwapImages[i], ToVkFormat(mFormat), i, usage, image_info);
        }

        if (vkSetDebugUtilsObjectNameEXT) {
            const VkDebugUtilsObjectNameInfoEXT nameInfoo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                .objectHandle = eastl::bit_cast<uint64_t>(mSwapChain),
                .pObjectName = mInfo.name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameInfoo);
        }
    }
    void VulkanSwapChain::CreateSemaphores() {
        mImageAcquireSemaphores.resize(mInfo.bufferCount);
        for (i32 i = 0; i < mInfo.bufferCount; ++i) {
            VkSemaphoreCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
            };

            VkResult result = vkCreateSemaphore(mDevice->GetVkDevice(),
                &createInfo, mDevice->Context()->GetVkAllocator(), &mImageAcquireSemaphores[i]);
            CheckVkResult(result);

            if (vkSetDebugUtilsObjectNameEXT) {
                eastl::string name1 = mInfo.name + " Acquire Image Semaphore #" + eastl::to_string(i);
                VkDebugUtilsObjectNameInfoEXT semaphoreNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_SEMAPHORE,
                    .objectHandle = eastl::bit_cast<uint64_t>(mImageAcquireSemaphores[i]),
                    .pObjectName = name1.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &semaphoreNameInfo);
            }
        }
    }

    void VulkanSwapChain::TrySetPresentMode(SwapChainPresentMode presentMode) {
        auto it = eastl::find(mSupportInfo.presentModes.begin(), mSupportInfo.presentModes.end(), ToVkPresentMode(presentMode));
        if (it != mSupportInfo.presentModes.end()) {
            mPresentMode = presentMode;
        } else {
            // try fallbacks
            switch (presentMode) {
            case SwapChainPresentMode::LowLatency:
                TrySetPresentMode(SwapChainPresentMode::Tearing);
                break;
            default: // vsync is always supported
                mPresentMode = SwapChainPresentMode::VSync;
                break;
            }
        }
    }
} // namespace PyroshockStudios::RHIVulkan
