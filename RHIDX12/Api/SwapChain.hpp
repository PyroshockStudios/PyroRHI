#pragma once
#include <RHIDX12/Core.hpp>
#include <PyroRHI/Api/ISwapChain.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        class D3DSwapChain : public ISwapChain, DeleteCopy, DeleteMove {
        public:
            D3DSwapChain(D3DDevice* device, SwapChainInfo&& info);
            ~D3DSwapChain();

            Image GetBackBuffer(u32 imageIndex) override;
            Image AcquireNextImage() override;
            void Resize() override;
            void SetPresentMode(PresentMode presentMode) override;
            const SwapChainInfo& Info() const override;
            Extent2D GetSurfaceExtent() const override;
            Format GetFormat() const override;
            ColorSpace GetColorSpace() const override;

            IDXGISwapChain3* InternalSwapChain() {
                return mSwapChain.Get();
            }
            UINT mSyncInterval = 0;
        private:
            void DestroyImages();
            void GetImages();

            eastl::vector<Image> mWrappedBuffers{};
            SwapChainInfo mInfo = {};
            HANDLE mSwapWait = {};
            ComPtr<IDXGISwapChain3> mSwapChain = {};
            Format mFormat = {};
            D3DDevice* mDevice = {};
            u32 mImageIndex = 0;
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios