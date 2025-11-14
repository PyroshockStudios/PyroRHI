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

#include "DXShaderCompiler.hpp"
#include <dxcapi.h>
#include <libassert/assert.hpp>

// DxilContainer.h from DirectXShaderCompiler
#define DXIL_FOURCC(ch0, ch1, ch2, ch3)                         \
    ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 | \
        (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)

enum DxilFourCC {
    DFCC_Container = DXIL_FOURCC(
        'D', 'X', 'B',
        'C'), // for back-compat with tools that look for DXBC containers
    DFCC_ResourceDef = DXIL_FOURCC('R', 'D', 'E', 'F'),
    DFCC_InputSignature = DXIL_FOURCC('I', 'S', 'G', '1'),
    DFCC_OutputSignature = DXIL_FOURCC('O', 'S', 'G', '1'),
    DFCC_PatchConstantSignature = DXIL_FOURCC('P', 'S', 'G', '1'),
    DFCC_ShaderStatistics = DXIL_FOURCC('S', 'T', 'A', 'T'),
    DFCC_ShaderDebugInfoDXIL = DXIL_FOURCC('I', 'L', 'D', 'B'),
    DFCC_ShaderDebugName = DXIL_FOURCC('I', 'L', 'D', 'N'),
    DFCC_FeatureInfo = DXIL_FOURCC('S', 'F', 'I', '0'),
    DFCC_PrivateData = DXIL_FOURCC('P', 'R', 'I', 'V'),
    DFCC_RootSignature = DXIL_FOURCC('R', 'T', 'S', '0'),
    DFCC_DXIL = DXIL_FOURCC('D', 'X', 'I', 'L'),
    DFCC_PipelineStateValidation = DXIL_FOURCC('P', 'S', 'V', '0'),
    DFCC_RuntimeData = DXIL_FOURCC('R', 'D', 'A', 'T'),
    DFCC_ShaderHash = DXIL_FOURCC('H', 'A', 'S', 'H'),
    DFCC_ShaderSourceInfo = DXIL_FOURCC('S', 'R', 'C', 'I'),
    DFCC_ShaderPDBInfo = DXIL_FOURCC('P', 'D', 'B', 'I'),
    DFCC_CompilerVersion = DXIL_FOURCC('V', 'E', 'R', 'S'),
};

#undef DXIL_FOURCC
// DxilContainer.h

namespace PyroshockStudios {
    namespace RHIDX12 {
        static eastl::vector<D3D12_SIGNATURE_PARAMETER_DESC> ExtractReflectionSemantics(const ComPtr<ID3D12ShaderReflection>& reflector) {
            eastl::vector<D3D12_SIGNATURE_PARAMETER_DESC> list{};

            D3D12_SHADER_DESC shaderDesc = {};
            CheckD3DResult(reflector->GetDesc(&shaderDesc), "Failed to get D3D12_SHADER_DESC!");

            // Iterate over input parameters
            for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
                D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                CheckD3DResult(reflector->GetInputParameterDesc(i, &paramDesc), "Failed to get shader input parameter description!");
                list.emplace_back(paramDesc);
            }

            return list;
        }


        eastl::vector<D3D12_SIGNATURE_PARAMETER_DESC> ReflectSemanticsDXBC(ComPtr<ID3D12ShaderReflection>& reflector, LPCVOID shaderBytecode, SIZE_T bytecodeLength) {
            CheckD3DResult(D3DReflect(shaderBytecode, bytecodeLength, IID_PPV_ARGS(&reflector)), "Failed to reflect DXBC byte code!");
            return ExtractReflectionSemantics(reflector);
        }

        eastl::vector<D3D12_SIGNATURE_PARAMETER_DESC> ReflectSemanticsDXIL(ComPtr<ID3D12ShaderReflection>& reflector, LPCVOID shaderBytecode, SIZE_T bytecodeLength) {
            ComPtr<IDxcContainerReflection> pReflection;
            ComPtr<IDxcUtils> pDxcUtils;
            ComPtr<IDxcBlobEncoding> pBlob;

            CheckD3DResult(D3D12GetInterface(CLSID_DxcUtils, IID_PPV_ARGS(&pDxcUtils)), "Failed to get IDxcUtils!");

            CheckD3DResult(pDxcUtils->CreateBlob(shaderBytecode, static_cast<UINT32>(bytecodeLength), DFCC_DXIL, pBlob.GetAddressOf()), "Failed to create IDxcBlob!");

            UINT32 shaderIdx;
            CheckD3DResult(D3D12GetInterface(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection)), "Failed to get IDxcContainerReflection!");
            CheckD3DResult(pReflection->Load(pBlob.Get()), "Failed to load reflection blob!");
            CheckD3DResult(pReflection->FindFirstPartKind(DFCC_DXIL, &shaderIdx), "Failed to get DXIL reflection");
            CheckD3DResult(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&reflector)), "Failed to get ID3D12ShaderReflection");

            return ExtractReflectionSemantics(reflector);
        }


        eastl::string DissasembleDXBC(LPCVOID shaderBytecode, SIZE_T bytecodeLength) {
            ComPtr<ID3DBlob> disasmBlob = nullptr;
            CheckD3DResult(D3DDisassemble(
                               shaderBytecode,
                               bytecodeLength,
                               D3D_DISASM_DISABLE_DEBUG_INFO | D3D_DISASM_INSTRUCTION_ONLY,
                               nullptr,
                               &disasmBlob),
                "Failed to dissasemble D3DBlob!");
            return eastl::string(static_cast<const char*>(disasmBlob->GetBufferPointer()), disasmBlob->GetBufferSize());
        }

        eastl::string DissasembleDXIL(LPCVOID shaderBytecode, SIZE_T bytecodeLength) {
            ComPtr<IDxcUtils> pDxcUtils;
            ComPtr<IDxcCompiler> pDxcCompiler;
            ComPtr<IDxcBlobEncoding> pBlob;

            CheckD3DResult(D3D12GetInterface(CLSID_DxcUtils, IID_PPV_ARGS(&pDxcUtils)), "Failed to get IDxcUtils!");
            CheckD3DResult(D3D12GetInterface(CLSID_DxcCompiler, IID_PPV_ARGS(&pDxcCompiler)), "Failed to get IDxcCompiler!");

            CheckD3DResult(pDxcUtils->CreateBlob(shaderBytecode, static_cast<UINT32>(bytecodeLength), DFCC_DXIL, pBlob.GetAddressOf()), "Failed to create IDxcBlob!");
            
            CheckD3DResult(pDxcCompiler->Disassemble(pBlob.Get(), pBlob.GetAddressOf()), "Failed to create IDxcBlob!");

            return eastl::string(static_cast<const char*>(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios
