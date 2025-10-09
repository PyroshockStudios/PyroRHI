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

#include <RHIDX12/D3DContext.hpp>
#include "SwapChain.hpp"
#include "CommandQueue.hpp"
#include "Device.hpp"

#include <libassert/assert.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DSwapChain::D3DSwapChain(D3DDevice* device, SwapChainInfo&& info)
            : mDevice(device), mInfo(eastl::move(info)) {

            // Describe and create the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.BufferCount = mInfo.bufferCount;
            swapChainDesc.Width = mInfo.extent.x;
            swapChainDesc.Height = mInfo.extent.y;
            switch (mInfo.format) {
            case SwapChainFormat::Unorm8BitLDR:
                swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                mFormat = Format::RGBA8Unorm;
                break;
            case SwapChainFormat::Srgb8BitLDR:
                swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                mFormat = Format::RGBA8Srgb;
                break;
            case SwapChainFormat::Unorm10BitLDR:
                swapChainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                mFormat = Format::A2RGB10Unorm;
                break;
            case SwapChainFormat::Float16BitHDR:
                swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                mFormat = Format::RGBA16Sfloat;
                break;
            default:
                ASSERT(false, "invalid format!");
            }
            if (info.imageUsage & ImageUsageFlagBits::RENDER_TARGET || info.imageUsage & ImageUsageFlagBits::BLIT_DST) {
                swapChainDesc.BufferUsage |= DXGI_USAGE_RENDER_TARGET_OUTPUT;
            }
            if (info.imageUsage & ImageUsageFlagBits::TRANSFER_SRC) {
                swapChainDesc.BufferUsage |= DXGI_USAGE_READ_ONLY;
            }
            if (info.imageUsage & ImageUsageFlagBits::BLIT_SRC) {
                swapChainDesc.BufferUsage |= DXGI_USAGE_SHADER_INPUT;
            }
            if (info.imageUsage & ImageUsageFlagBits::UNORDERED_ACCESS) {
                swapChainDesc.BufferUsage |= DXGI_USAGE_UNORDERED_ACCESS;
            }
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            switch (info.alphaMode) {
            case SwapChainAlphaMode::None:
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                break;
            case SwapChainAlphaMode::Premultiplied:
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
                break;
            case SwapChainAlphaMode::Straight:
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_STRAIGHT;
                break;
            default:
                ASSERT(false, "Invalid alpha mode");
            }
            switch (info.presentMode) {
            case PresentMode::VSync:
            case PresentMode::VSyncAdaptive:
                mSyncInterval = 1;
                break;
            case PresentMode::Tearing:
                swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                mSyncInterval = 0;
                break;
            case PresentMode::LowLatency:
                mSyncInterval = 0;
                break;
            default:
                ASSERT(false, "Invalid present mode");
            }
            swapChainDesc.SampleDesc.Count = 1;
            HWND hwnd = reinterpret_cast<HWND>(info.nativeWindow);

            ComPtr<IDXGISwapChain1> swapChain;
            if (info.alphaMode == SwapChainAlphaMode::None) {
                CheckD3DResult(mDevice->InternalFactory()->CreateSwapChainForHwnd(
                    static_cast<D3DCommandQueue*>(mDevice->GetPresentQueue())->InternalQueue(), // Swap chain needs the queue so that it can force a flush on it.
                    hwnd,
                    &swapChainDesc,
                    nullptr,
                    nullptr,
                    &swapChain));
                gDx12Context->FlushDebugMessages();
            } else {
                CheckD3DResult(DCompositionCreateDevice(
                    nullptr,
                    __uuidof(IDCompositionDevice),
                    reinterpret_cast<void**>(mDcompDevice.GetAddressOf())));

                CheckD3DResult(mDcompDevice->CreateTargetForHwnd(hwnd, TRUE, &mDcompTarget));

                CheckD3DResult(mDcompDevice->CreateVisual(&mDcompVisual));

                // Attach swapchain to the visual
                CheckD3DResult(mDevice->InternalFactory()->CreateSwapChainForComposition(
                    static_cast<D3DCommandQueue*>(mDevice->GetPresentQueue())->InternalQueue(),
                    &swapChainDesc,
                    nullptr,
                    &swapChain));

                CheckD3DResult(mDcompVisual->SetContent(swapChain.Get()));
                // Add visual to target
                CheckD3DResult(mDcompTarget->SetRoot(mDcompVisual.Get()));
                // Commit composition
                CheckD3DResult(mDcompDevice->Commit());
                gDx12Context->FlushDebugMessages();
            }

            CheckD3DResult(swapChain.As(&mSwapChain));
            D3DSetDebugName(mSwapChain, info.name.c_str());

            mSwapWait = mSwapChain->GetFrameLatencyWaitableObject();
            CheckD3DResult(mSwapChain->SetMaximumFrameLatency(1));
            gDx12Context->FlushDebugMessages();
            GetImages();
        }
        D3DSwapChain::~D3DSwapChain() {
            DestroyImages();
        }

        Image D3DSwapChain::GetBackBuffer(u32 imageIndex) {
            return mWrappedBuffers[imageIndex];
        }

        Image D3DSwapChain::AcquireNextImage() {
            WaitForSingleObjectEx(mSwapWait, INFINITE, TRUE);
            gDx12Context->FlushDebugMessages();
            mImageIndex = mSwapChain->GetCurrentBackBufferIndex();
            gDx12Context->FlushDebugMessages();
            return mWrappedBuffers[mImageIndex];
        }

        void D3DSwapChain::Resize() {
            DestroyImages();
            HWND window = reinterpret_cast<HWND>(mInfo.nativeWindow);
            RECT area;
            GetClientRect(window, &area);
            mInfo.extent.x = area.right;
            mInfo.extent.y = area.bottom;

            DXGI_SWAP_CHAIN_DESC1 desc;
            CheckD3DResult(mSwapChain->GetDesc1(&desc));
            auto q = static_cast<D3DCommandQueue*>(mDevice->GetPresentQueue())->InternalQueue();
            // IUnknown* queueArray[1] = { q };
            // UINT nodeMask = 1;
            // CheckD3DResult(mSwapChain->ResizeBuffers1(desc.BufferCount, mInfo.extent.x, mInfo.extent.y, desc.Format, desc.Flags, &nodeMask, queueArray));
            mSwapChain->ResizeBuffers(desc.BufferCount, mInfo.extent.x, mInfo.extent.y, desc.Format, desc.Flags);
            gDx12Context->FlushDebugMessages();
            GetImages();
        }

        void D3DSwapChain::SetPresentMode(PresentMode presentMode) {
            ASSERT(false, "TODO");
        }

        const SwapChainInfo& D3DSwapChain::Info() const {
            return mInfo;
        }
        Extent2D D3DSwapChain::GetSurfaceExtent() const {
            return mInfo.extent;
        }
        Format D3DSwapChain::GetFormat() const {
            return mFormat;
        }
        ColorSpace D3DSwapChain::GetColorSpace() const {
            ASSERT(false, "TODO");
            return ColorSpace();
        }

        void D3DSwapChain::DestroyImages() {
            for (Image img : mWrappedBuffers) {
                mDevice->ResourcePool().ReleaseImage(img);
                gDx12Context->FlushDebugMessages();
            }
            mWrappedBuffers.clear();
        }

        void D3DSwapChain::GetImages() {
            mWrappedBuffers.resize(mInfo.bufferCount);
            for (UINT i = 0; i < mWrappedBuffers.size(); ++i) {
                auto [image, data] = mDevice->ResourcePool().AllocImage();
                CheckD3DResult(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&data.resource)));
                gDx12Context->FlushDebugMessages();
                data.info = {
                    .dimensions = 2,
                    .format = mFormat,
                    .size = { mInfo.extent.x, mInfo.extent.y },
                    .usage = mInfo.imageUsage,
                    .name = mInfo.name + " DXGI Swap Buffer #" + eastl::to_string(i),
                };
                data.lastValidStates.resize(1, D3D12_RESOURCE_STATE_COMMON);
                D3DSetDebugName(data.resource, data.info.name.c_str());
                mDevice->ImageAddIfNecessaryBlitSupport(data);
                mWrappedBuffers[i] = image;
            }
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios