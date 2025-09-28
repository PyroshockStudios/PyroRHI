#pragma once
#include <RHIVulkan/Core.hpp>

#include <RHIVulkan/Api/Device.hpp>
#include <PyroRHI/Api/ISwapChain.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanSwapChain : public ISwapChain, DeleteCopy, DeleteMove {
        public:
            VulkanSwapChain(VulkanDevice* device, const SwapChainInfo& info);
            ~VulkanSwapChain();

            Image GetBackBuffer(u32 imageIndex) override;
            Image AcquireNextImage() override;

            void Resize() override;
            void SetPresentMode(PresentMode presentMode) override;
            const SwapChainInfo& Info() const override;
            Extent2D GetSurfaceExtent() const override;
            Format GetFormat() const override;
            ColorSpace GetColorSpace() const override;

            u32 GetCurrentImageIndex() {
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

        private:
            void CreateSurface();
            void CreateSwapChain(VkSwapchainKHR oldSwapChain);
            void CreateSemaphores();
            void TrySetPresentMode(PresentMode presentMode);

            VulkanDevice* mDevice;

            VulkanSwapChainSupportInfo mSupportInfo{};
            SwapChainInfo mInfo = {};

            VkSurfaceKHR mSurface = {};

            VkSwapchainKHR mSwapChain = {};
            eastl::vector<VkImage> mSwapImages = {};
            eastl::vector<Image> mWrappedImages = {};

            i32 mImageAcquireIndex = -1;
            eastl::vector<VkSemaphore> mImageAcquireSemaphores = {};

            u32 mImageIndex = 0;

            PresentMode mPresentMode = {};
            Format mFormat = {};
            ColorSpace mColorSpace = {};
        };

    } // namespace RHIVulkan
} // namespace PyroshockStudios