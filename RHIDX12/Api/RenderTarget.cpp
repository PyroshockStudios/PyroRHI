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
namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DRenderTarget::D3DRenderTarget(D3DDevice* device, bool bDsv, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, RenderTargetInfo&& info)
            : mDevice(device), bDsv(bDsv), mRenderTarget(descriptor), mInfo(eastl::move(info)) {

            if (bDsv) {
                D3D12_DEPTH_STENCIL_VIEW_DESC view = {};
                const auto& imageData = mDevice->ResourcePool().Get(mInfo.image);
                view.Format = ToDXGIFormat(imageData.info.format);
                if (mInfo.slice.arrayLayer == 0) {
                    view.ViewDimension = imageData.info.sampleCount == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    view.Texture2D.MipSlice = mInfo.slice.mipLevel;
                    mSubresource = D3D12CalcSubresource(view.Texture2D.MipSlice, 0, 0, imageData.info.mipLevelCount, imageData.info.arrayLayerCount);
                } else {
                    view.ViewDimension = imageData.info.sampleCount == 1 ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    view.Texture2DArray.MipSlice = mInfo.slice.mipLevel;
                    view.Texture2DArray.FirstArraySlice = mInfo.slice.arrayLayer;
                    view.Texture2DArray.ArraySize = 1;
                    mSubresource = D3D12CalcSubresource(view.Texture2D.MipSlice, view.Texture2DArray.FirstArraySlice, 0, imageData.info.mipLevelCount, imageData.info.arrayLayerCount);
                }
                mDevice->InternalDevice()->CreateDepthStencilView(mDevice->ResourcePool().Get(info.image).resource.Get(), &view, mRenderTarget);
            } else {
                D3D12_RENDER_TARGET_VIEW_DESC view = {};
                const auto& imageData = mDevice->ResourcePool().Get(mInfo.image);
                view.Format = ToDXGIFormat(imageData.info.format);

                if (mInfo.slice.arrayLayer == 0) {
                    view.ViewDimension = imageData.info.sampleCount == 1 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    view.Texture2D.MipSlice = mInfo.slice.mipLevel;
                    mSubresource = D3D12CalcSubresource(view.Texture2D.MipSlice, 0, 0, imageData.info.mipLevelCount, imageData.info.arrayLayerCount);
                } else {
                    view.ViewDimension = imageData.info.sampleCount == 1 ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    view.Texture2DArray.MipSlice = mInfo.slice.mipLevel;
                    view.Texture2DArray.FirstArraySlice = mInfo.slice.arrayLayer;
                    view.Texture2DArray.ArraySize = 1;
                    mSubresource = D3D12CalcSubresource(view.Texture2D.MipSlice, view.Texture2DArray.FirstArraySlice, 0, imageData.info.mipLevelCount, imageData.info.arrayLayerCount);
                }
                mDevice->InternalDevice()->CreateRenderTargetView(mDevice->ResourcePool().Get(info.image).resource.Get(), &view, mRenderTarget);
            }
        }
        D3DRenderTarget::~D3DRenderTarget() {
        }

        const RenderTargetInfo& D3DRenderTarget::Info() const {
            return mInfo;
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios