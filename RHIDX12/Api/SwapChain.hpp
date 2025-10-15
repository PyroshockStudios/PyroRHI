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
#include <RHIDX12/Core.hpp>
#include <PyroRHI/Api/ISwapChain.hpp>
#include <dcomp.h>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        class D3DSwapChain : public ISwapChain, DeleteCopy, DeleteMove {
        public:
            D3DSwapChain(D3DDevice* device, SwapChainInfo&& info);
            ~D3DSwapChain();

            Image GetBackBuffer(i32 imageIndex) override;
            i32 AcquireNextImage() override;
            void Resize() override;
            void SetPresentMode(SwapChainPresentMode presentMode) override;
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
            ComPtr<IDCompositionDevice> mDcompDevice = {};
            ComPtr<IDCompositionTarget> mDcompTarget = {};
            ComPtr<IDCompositionVisual> mDcompVisual = {};

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