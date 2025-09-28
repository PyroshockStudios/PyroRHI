#pragma once
#include <RHIDX12/Core.hpp>
#include <PyroRHI/Api/RenderTarget.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        class D3DRenderTarget : DeleteCopy, DeleteMove {
        public:
            D3DRenderTarget(D3DDevice* device, bool bDsv, D3D12_CPU_DESCRIPTOR_HANDLE descriptor, RenderTargetInfo&& info);
            ~D3DRenderTarget();

            const RenderTargetInfo& Info() const;

            D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() {
                return mRenderTarget;
            }
            UINT GetSubresource() {
                return mSubresource;
            }

            bool IsDSV() {
                return bDsv;
            }

        private:
            RenderTargetInfo mInfo = {};
            UINT mSubresource = {};
            D3D12_CPU_DESCRIPTOR_HANDLE mRenderTarget = {};
            D3DDevice* mDevice = {};
            bool bDsv = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios