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