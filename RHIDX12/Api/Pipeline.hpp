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