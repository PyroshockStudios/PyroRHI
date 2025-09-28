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