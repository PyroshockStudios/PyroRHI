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
#include <RHIVulkan/Core.hpp>

#include <PyroRHI/Api/ISwapChain.hpp>
#include <RHIVulkan/Api/Device.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanSwapChain : public ISwapChain, DeleteCopy, DeleteMove {
        public:
            VulkanSwapChain(VulkanDevice* device, const SwapChainInfo& info);
            ~VulkanSwapChain();

            Image GetBackBuffer(u32 imageIndex) override;
            u32 AcquireNextImage() override;

            void Resize() override;
            void SetPresentMode(SwapChainPresentMode presentMode) override;
            const SwapChainInfo& Info() const override;
            Extent2D GetSurfaceExtent() const override;
            Format GetFormat() const override;
            ColorSpace GetColorSpace() const override;

            u32 GetCurrentImageIndex() const override {
                return mImageIndex;
            }
            VkSwapchainKHR GetVkSwapChain() {
                return mSwapChain;
            }
            VkSurfaceKHR GetVkSurface() {
                return mSurface;
            }
            VkSemaphore GetCurrentImageAcquireSemaphore() {
                return mImageAcquireSemaphores[mImageAcquireIndex];
            }
            VkSemaphore GetCurrentRenderFinishSemaphore() {
                return mRenderFinishSemaphores[mImageIndex];
            }

        private:
            void CreateSurface();
            void CreateSwapChain(VkSwapchainKHR oldSwapChain);
            void CreateSync();
            void TrySetPresentMode(SwapChainPresentMode presentMode);

            VulkanDevice* mDevice;

            VulkanSwapChainSupportInfo mSupportInfo{};
            SwapChainInfo mInfo = {};

            VkSurfaceKHR mSurface = {};

            VkSwapchainKHR mSwapChain = {};
            eastl::vector<VkImage> mSwapImages = {};
            eastl::vector<Image> mWrappedImages = {};

            i32 mImageAcquireIndex = -1;
            eastl::vector<VkSemaphore> mImageAcquireSemaphores = {};
            eastl::vector<VkSemaphore> mRenderFinishSemaphores = {};
            VkFence mSwapchainAcquireFence = VK_NULL_HANDLE;

            u32 mImageIndex = 0;

            SwapChainPresentMode mPresentMode = {};
            Format mFormat = {};
            ColorSpace mColorSpace = {};
        };

    } // namespace RHIVulkan
} // namespace PyroshockStudios