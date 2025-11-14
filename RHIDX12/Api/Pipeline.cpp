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

#include "Pipeline.hpp"
#include "Device.hpp"
#include <RHIDX12/Helper/DXShaderCompiler.hpp>

// temp
#include <iostream>
#include <libassert/assert.hpp>

#include <PyroCommon/Util/String.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        using OWORD = eastl::array<u32, 4>;

        void PopulateSpecialisationConstants(eastl::span<const u8> code, const eastl::span<const SpecializationConstantInfo>& list,
            ID3D12Device* device, ComPtr<ID3D12Resource>& outResource, bool bDxil) {
            eastl::vector<OWORD> shaderSpecializationData{};
            for (const auto& info : list) {
                if ((shaderSpecializationData.size()) < (info.location + 1)) {
                    shaderSpecializationData.resize(info.location + 1);
                }
                shaderSpecializationData[info.location][0] = info.data.Get<u32>();
                shaderSpecializationData[info.location][1] = 1;
            }

            // HACK: There is no clean way to get the variable names originally declared,
            if (bDxil) {
                eastl::string disasm = DissasembleDXIL(code.data(), code.size());
                std::cout << disasm.c_str() << std::endl;

                shaderSpecializationData.resize(Limits::MAX_SPECIALIZATION_CONSTANTS);
            } else {
                // FXC seems to emit CB0[BINDING][OWORD offset], so we can safely search for this string
                eastl::string disasm = DissasembleDXBC(code.data(), code.size());
                for (i32 i = static_cast<i32>(shaderSpecializationData.size()); i < Limits::MAX_SPECIALIZATION_CONSTANTS; ++i) {
                    eastl::string name = "CB0[8][" + eastl::to_string(i) + "].x";
                    if (disasm.find(name.c_str(), 0, name.size()) != eastl::string_view::npos) {
                        if ((shaderSpecializationData.size()) < (i + 1)) {
                            shaderSpecializationData.resize(i + 1);
                        }
                    }
                }
            }

            if (shaderSpecializationData.size() > 0) {
                D3D12_RESOURCE_DESC desc = {};
                desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                desc.Width = shaderSpecializationData.size() * sizeof(*shaderSpecializationData.data());
                desc.Height = 1;
                desc.DepthOrArraySize = 1;
                desc.MipLevels = 1;
                desc.Format = DXGI_FORMAT_UNKNOWN;
                desc.SampleDesc.Count = 1;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD /*TODO: D3D12_HEAP_TYPE_GPU_UPLOAD*/);
                device->CreateCommittedResource(
                    &heap,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_GENERIC_READ, // must be GENERIC_READ for upload heaps
                    nullptr,
                    IID_PPV_ARGS(&outResource));
                D3D12_RANGE range{};
                range.Begin = 0;
                range.End = desc.Width;
                void* ptr;
                outResource->Map(0, &range, &ptr);
                memcpy(ptr, shaderSpecializationData.data(), desc.Width);
                outResource->Unmap(0, &range);
            }
        };

        D3DRasterPipeline::D3DRasterPipeline(D3DDevice* device, const RasterPipelineInfo& info, ID3D12RootSignature* rootSig, const RasterPipelineShaderStages& stages)
            : mDevice(device), mInfo(info) {

            const bool bDxil = mDevice->mActiveShaderModel >= 0x60;

            if (stages.vertexShaderInfo) {
                PopulateSpecialisationConstants(stages.vertexShaderInfo->program, stages.vertexShaderInfo->specializationConstants,
                    mDevice->InternalDevice(), mEmulatedSpecialisationConstants[0], bDxil);
            }
            if (stages.hullShaderInfo) {
                PopulateSpecialisationConstants(stages.hullShaderInfo->program, stages.hullShaderInfo->specializationConstants,
                    mDevice->InternalDevice(), mEmulatedSpecialisationConstants[1], bDxil);
            }
            if (stages.domainShaderInfo) {
                PopulateSpecialisationConstants(stages.domainShaderInfo->program, stages.domainShaderInfo->specializationConstants,
                    mDevice->InternalDevice(), mEmulatedSpecialisationConstants[2], bDxil);
            }
            if (stages.geometryShaderInfo) {
                PopulateSpecialisationConstants(stages.geometryShaderInfo->program, stages.geometryShaderInfo->specializationConstants,
                    mDevice->InternalDevice(), mEmulatedSpecialisationConstants[3], bDxil);
            }
            if (stages.fragmentShaderInfo) {
                PopulateSpecialisationConstants(stages.fragmentShaderInfo->program, stages.fragmentShaderInfo->specializationConstants,
                    mDevice->InternalDevice(), mEmulatedSpecialisationConstants[4], bDxil);
            }


            // --- Input Layout ---
            eastl::vector<D3D12_SIGNATURE_PARAMETER_DESC> semantics{};
            ComPtr<ID3D12ShaderReflection> reflector = nullptr;
            if (stages.vertexShaderInfo && !info.inputAssemblyState.vertexAttributes.empty()) {
                if (bDxil) {
                    semantics = ReflectSemanticsDXIL(reflector, stages.vertexShaderInfo->program.data(), stages.vertexShaderInfo->program.size());
                } else {
                    semantics = ReflectSemanticsDXBC(reflector, stages.vertexShaderInfo->program.data(), stages.vertexShaderInfo->program.size());
                }
            }
            eastl::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
            for (u32 i = 0; i < info.inputAssemblyState.vertexAttributes.size(); ++i) {
                auto& attr = info.inputAssemblyState.vertexAttributes[i];
                auto bindingIt = eastl::find_if(info.inputAssemblyState.vertexBindings.begin(), info.inputAssemblyState.vertexBindings.end(),
                    [&](const auto& b) { return b.binding == attr.binding; });
                ASSERT(bindingIt != info.inputAssemblyState.vertexBindings.end(), "Vertex input binding is out of bounds!");
                auto& bind = *bindingIt;
                ASSERT(attr.location < semantics.size(), "Vertex input location is out of bounds!");
                inputElements.push_back({
                    /* SemanticName    */ semantics[attr.location].SemanticName,
                    /* SemanticIndex   */ semantics[attr.location].SemanticIndex,
                    /* Format          */ ToDXGIFormat(attr.format),
                    /* InputSlot       */ attr.binding,
                    /* AlignedByteOffset */ attr.offset,
                    /* InputSlotClass   */ bind.bPerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    /* InstanceDataStepRate */ bind.bPerInstance ? 1U : 0U,
                });
                ASSERT(inputElements.back().Format != DXGI_FORMAT_UNKNOWN, "BAD VERTEX FORMAT!");
                if (mVertexBufferStrides.size() < (attr.binding + 1)) {
                    mVertexBufferStrides.resize(attr.binding + 1);
                }
                mVertexBufferStrides[attr.binding] = bind.stride;
            }

            D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
            if (!inputElements.empty()) {
                inputLayoutDesc = {
                    .pInputElementDescs = inputElements.data(),
                    .NumElements = (UINT)inputElements.size(),
                };
            }

            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
            if (info.inputAssemblyState.bPrimitiveRestart) {
                ibStripCutValue = info.inputAssemblyState.indexType ==
                                          IndexType::Uint16
                                      ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF
                                      : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
            }

            // --- Rasterizer State ---
            const auto& rast = info.rasterizerState;
            D3D12_RASTERIZER_DESC rastDesc = {};
            rastDesc.FillMode = rast.polygonMode == PolygonMode::Line ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
            rastDesc.CullMode = ToD3D12CullMode(rast.faceCulling);
            rastDesc.FrontCounterClockwise = rast.frontFaceWinding == WindingOrder::CounterClockwise;
            rastDesc.DepthBias = rast.bDepthBias ? (INT)rast.depthBiasConstantFactor : 0;
            rastDesc.DepthBiasClamp = rast.depthBiasClamp;
            rastDesc.SlopeScaledDepthBias = rast.depthBiasSlopeFactor;
            rastDesc.DepthClipEnable = rast.bDepthClamp ? FALSE : TRUE;
            rastDesc.MultisampleEnable = info.multiSampleState.sampleCount != RasterizationSamples::e1;
            rastDesc.AntialiasedLineEnable = (!rastDesc.MultisampleEnable && info.rasterizerState.lineMode == LineMode::Smooth) ? TRUE : FALSE;
            rastDesc.ForcedSampleCount = 0;
            rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

            // --- Blend State ---
            D3D12_BLEND_DESC blendDesc = {};
            blendDesc.AlphaToCoverageEnable = info.multiSampleState.bAlphaToCoverage;
            blendDesc.IndependentBlendEnable = info.colorTargetStates.size() > 1;
            for (size_t i = 0; i < info.colorTargetStates.size(); ++i) {
                const auto& rt = info.colorTargetStates[i];
                auto& rtBlend = blendDesc.RenderTarget[i];
                rtBlend.RenderTargetWriteMask = static_cast<UINT8>(rt.writeMask.data);

                if (rt.blend.has_value()) {
                    rtBlend.BlendEnable = TRUE;
                    const auto& blend = rt.blend.value();
                    rtBlend.SrcBlend = ToD3D12BlendFactor(blend.srcColorBlendFactor);
                    rtBlend.DestBlend = ToD3D12BlendFactor(blend.dstColorBlendFactor);
                    rtBlend.BlendOp = ToD3D12BlendOp(blend.colorBlendOp);
                    rtBlend.SrcBlendAlpha = ToD3D12BlendFactor(blend.srcAlphaBlendFactor);
                    rtBlend.DestBlendAlpha = ToD3D12BlendFactor(blend.dstAlphaBlendFactor);
                    rtBlend.BlendOpAlpha = ToD3D12BlendOp(blend.alphaBlendOp);
                } else {
                    rtBlend.BlendEnable = FALSE;
                }
            }

            // --- Depth Stencil ---
            D3D12_DEPTH_STENCIL_DESC dsDesc = {};
            if (info.depthStencilState.has_value()) {
                const auto& ds = info.depthStencilState.value();
                dsDesc.DepthEnable = ds.depthTestState != DepthStencilTestState::Ignore;
                dsDesc.DepthWriteMask = (ds.depthTestState == DepthStencilTestState::ReadWrite) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
                dsDesc.DepthFunc = ToD3D12CompareOp(ds.depthTest.compareOp);
                dsDesc.StencilEnable = ds.stencilTestState != DepthStencilTestState::Ignore;
                dsDesc.StencilReadMask = (UINT8)ds.frontStencilTest.compareMask;
                dsDesc.StencilWriteMask = (UINT8)ds.frontStencilTest.writeMask;
                dsDesc.FrontFace.StencilFailOp = ToD3D12StencilOp(ds.frontStencilTest.failOp);
                dsDesc.FrontFace.StencilDepthFailOp = ToD3D12StencilOp(ds.frontStencilTest.depthFailOp);
                dsDesc.FrontFace.StencilPassOp = ToD3D12StencilOp(ds.frontStencilTest.passOp);
                dsDesc.FrontFace.StencilFunc = ToD3D12CompareOp(ds.frontStencilTest.compareOp);
                dsDesc.BackFace.StencilFailOp = ToD3D12StencilOp(ds.backStencilTest.failOp);
                dsDesc.BackFace.StencilDepthFailOp = ToD3D12StencilOp(ds.backStencilTest.depthFailOp);
                dsDesc.BackFace.StencilPassOp = ToD3D12StencilOp(ds.backStencilTest.passOp);
                dsDesc.BackFace.StencilFunc = ToD3D12CompareOp(ds.backStencilTest.compareOp);
            }

            // --- Topology ---
            D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = ToD3D12PrimitiveTopologyType(info.inputAssemblyState.primitiveTopology);
            // For some reason this is set as a dynamic PSO
            mTopology = ToD3D12PrimitiveTopology(info.inputAssemblyState.primitiveTopology, info.tesselationState ? info.tesselationState->controlPoints : 0);

            // --- Render Targets ---
            D3D12_RT_FORMAT_ARRAY rtvFormats = {};
            rtvFormats.NumRenderTargets = (UINT)info.colorTargetStates.size();
            for (size_t i = 0; i < info.colorTargetStates.size(); ++i) {
                rtvFormats.RTFormats[i] = ToDXGIFormat(info.colorTargetStates[i].format);
            }
            DXGI_FORMAT dsvFormat = info.depthStencilState.has_value() ? ToDXGIFormat(info.depthStencilState->depthStencilFormat) : DXGI_FORMAT_UNKNOWN;

            // --- Multisampling ---
            DXGI_SAMPLE_DESC sampleDesc = {};
            sampleDesc.Count = (UINT)info.multiSampleState.sampleCount;
            sampleDesc.Quality = 0;


            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.pRootSignature = rootSig;
            if (stages.vertexShaderInfo) {
                psoDesc.VS = { stages.vertexShaderInfo->program.data(), stages.vertexShaderInfo->program.size() };
            }
            if (stages.hullShaderInfo) {
                psoDesc.HS = { stages.hullShaderInfo->program.data(), stages.hullShaderInfo->program.size() };
            }
            if (stages.domainShaderInfo) {
                psoDesc.DS = { stages.domainShaderInfo->program.data(), stages.domainShaderInfo->program.size() };
            }
            if (stages.geometryShaderInfo) {
                psoDesc.GS = { stages.geometryShaderInfo->program.data(), stages.geometryShaderInfo->program.size() };
            }
            if (stages.fragmentShaderInfo) {
                psoDesc.PS = { stages.fragmentShaderInfo->program.data(), stages.fragmentShaderInfo->program.size() };
            }
            psoDesc.InputLayout = inputLayoutDesc;
            psoDesc.IBStripCutValue = ibStripCutValue;
            psoDesc.RasterizerState = rastDesc;
            psoDesc.SampleMask = 0xFFFFFFFFU;
            psoDesc.BlendState = blendDesc;
            psoDesc.DepthStencilState = dsDesc;
            psoDesc.PrimitiveTopologyType = topologyType;
            psoDesc.NumRenderTargets = rtvFormats.NumRenderTargets;
            memcpy(psoDesc.RTVFormats, rtvFormats.RTFormats, sizeof(psoDesc.RTVFormats));
            psoDesc.DSVFormat = dsvFormat;
            psoDesc.SampleDesc = sampleDesc;

            CheckD3DResult(mDevice->InternalDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)), "Failed to create Graphics ID3D12PipelineState!");

            D3DSetDebugName(mPipelineState, info.name.c_str());
        }
        D3DRasterPipeline::~D3DRasterPipeline() {
        }

        D3DComputePipeline::D3DComputePipeline(D3DDevice* device, const ComputePipelineInfo& info, ID3D12RootSignature* rootSig, const ShaderInfo& stage)
            : mDevice(device), mInfo(info) {

            const bool bDxil = mDevice->mActiveShaderModel >= 0x60;

            PopulateSpecialisationConstants(stage.program, stage.specializationConstants,
                mDevice->InternalDevice(), mEmulatedSpecialisationConstant, bDxil);


            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
            psoDesc.CS = { stage.program.data(), stage.program.size() };
            psoDesc.pRootSignature = rootSig;

            CheckD3DResult(mDevice->InternalDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)), "Failed to create Compute ID3D12PipelineState!");

            D3DSetDebugName(mPipelineState, info.name.c_str());
        }

        D3DComputePipeline::~D3DComputePipeline() {
        }

    } // namespace RHIDX12
} // namespace PyroshockStudios
