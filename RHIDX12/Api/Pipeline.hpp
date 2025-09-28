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
#include <PyroRHI/Api/Pipeline.hpp>
namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        class D3DRasterPipeline : DeleteCopy, DeleteMove {
        public:
            D3DRasterPipeline(D3DDevice* device, const RasterPipelineInfo& info, ID3D12RootSignature* rootSig, const RasterPipelineShaderStages& stages);
            ~D3DRasterPipeline();

            ComPtr<ID3D12PipelineState> mPipelineState = {};
            D3D_PRIMITIVE_TOPOLOGY mTopology = {};
            eastl::vector<UINT> mVertexBufferStrides{};
            eastl::array<ComPtr<ID3D12Resource>, 5> mEmulatedSpecialisationConstants{};

            const RasterPipelineInfo& Info() {
                return mInfo;
            }

        private:
            D3DDevice* mDevice = {};
            RasterPipelineInfo mInfo;
        };
        class D3DComputePipeline : DeleteCopy, DeleteMove {
        public:
            D3DComputePipeline(D3DDevice* device, const ComputePipelineInfo& info, ID3D12RootSignature* rootSig, const ShaderInfo& stage);
            ~D3DComputePipeline();

            ComPtr<ID3D12PipelineState> mPipelineState = {};
            ComPtr<ID3D12Resource> mEmulatedSpecialisationConstant{};

            const ComputePipelineInfo& Info() {
                return mInfo;
            }

        private:
            D3DDevice* mDevice = {};
            ComputePipelineInfo mInfo;
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios