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
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <directx/d3dx12.h>

#include <PyroCommon/Core.hpp>
#include <PyroCommon/LoggerInterface.hpp>
#include <PyroRHI/Api/Util.hpp>

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
// #include <D3Dcompiler.h>
// #include <DirectXMath.h>
#include <shellapi.h>
#include <wrl.h>
#undef CreateSemaphore
#undef CreateEvent
#undef OPAQUE
using Microsoft::WRL::ComPtr;

typedef void(WINAPI* PFN_BeginEventOnCommandList)(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
typedef void(WINAPI* PFN_EndEventOnCommandList)(ID3D12GraphicsCommandList* commandList);
typedef void(WINAPI* PFN_SetMarkerOnCommandList)(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);

extern PFN_BeginEventOnCommandList gPixBeginEventOnCommandListFn;
extern PFN_EndEventOnCommandList gPixEndEventOnCommandListFn;
extern PFN_SetMarkerOnCommandList gPixSetMarkerOnCommandListFn;

namespace PyroshockStudios {
    namespace RHIDX12 {
        extern ILogStream* gDX12Sink;

#define D3DSetDebugName(interf, str)                                                                \
    do {                                                                                            \
        if (interf)                                                                                 \
            interf->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(str)), str); \
    } while (false)

        inline eastl::string WideToUTF8(const WCHAR* wideStr) {
            if (!wideStr)
                return {};

            int requiredSize = ::WideCharToMultiByte(
                CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
            if (requiredSize <= 0)
                return {};

            eastl::string result;
            result.resize(static_cast<usize>(requiredSize - 1)); // exclude null terminator
            ::WideCharToMultiByte(
                CP_UTF8, 0, wideStr, -1, result.data(), requiredSize, nullptr, nullptr);
            return result;
        }

        void CheckD3DResult(HRESULT result);

        PYRO_FORCEINLINE static constexpr DXGI_FORMAT ToDXGIFormat(Format type) {
            switch (type) {
                // case Format::RGBA4Unorm:
            case Format::BGRA4Unorm:
                return DXGI_FORMAT_B4G4R4A4_UNORM;
                // case Format::RGB565Unorm:
            case Format::BGR565Unorm:
                return DXGI_FORMAT_B5G6R5_UNORM;
                // case Format::RGB5A1Unorm:
            case Format::BGR5A1Unorm:
                return DXGI_FORMAT_B5G5R5A1_UNORM;
            case Format::R8Unorm:
                return DXGI_FORMAT_R8_UNORM;
            case Format::R8Snorm:
                return DXGI_FORMAT_R8_SNORM;
            case Format::R8Uint:
                return DXGI_FORMAT_R8_UINT;
            case Format::R8Sint:
                return DXGI_FORMAT_R8_SINT;
            case Format::RG8Unorm:
                return DXGI_FORMAT_R8G8_UNORM;
            case Format::RG8Snorm:
                return DXGI_FORMAT_R8G8_SNORM;
            case Format::RG8Uint:
                return DXGI_FORMAT_R8G8_UINT;
            case Format::RG8Sint:
                return DXGI_FORMAT_R8G8_SINT;
            case Format::RGBA8Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Format::RGBA8Snorm:
                return DXGI_FORMAT_R8G8B8A8_SNORM;
            case Format::RGBA8Uint:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case Format::RGBA8Sint:
                return DXGI_FORMAT_R8G8B8A8_SINT;
            case Format::RGBA8Srgb:
                return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case Format::BGRA8Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::BGRA8Srgb:
                return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case Format::A2RGB10Unorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Format::A2RGB10Uint:
                return DXGI_FORMAT_R10G10B10A2_UINT;
            case Format::R16Unorm:
                return DXGI_FORMAT_R16_UNORM;
            case Format::R16Snorm:
                return DXGI_FORMAT_R16_SNORM;
            case Format::R16Uint:
                return DXGI_FORMAT_R16_UINT;
            case Format::R16Sint:
                return DXGI_FORMAT_R16_SINT;
            case Format::R16Sfloat:
                return DXGI_FORMAT_R16_FLOAT;
            case Format::RG16Unorm:
                return DXGI_FORMAT_R16G16_UNORM;
            case Format::RG16Snorm:
                return DXGI_FORMAT_R16G16_SNORM;
            case Format::RG16Uint:
                return DXGI_FORMAT_R16G16_UINT;
            case Format::RG16Sint:
                return DXGI_FORMAT_R16G16_SINT;
            case Format::RG16Sfloat:
                return DXGI_FORMAT_R16G16_FLOAT;
            case Format::RGBA16Unorm:
                return DXGI_FORMAT_R16G16B16A16_UNORM;
            case Format::RGBA16Snorm:
                return DXGI_FORMAT_R16G16B16A16_SNORM;
            case Format::RGBA16Uint:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            case Format::RGBA16Sint:
                return DXGI_FORMAT_R16G16B16A16_SINT;
            case Format::RGBA16Sfloat:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case Format::R32Uint:
                return DXGI_FORMAT_R32_UINT;
            case Format::R32Sint:
                return DXGI_FORMAT_R32_SINT;
            case Format::R32Sfloat:
                return DXGI_FORMAT_R32_FLOAT;
            case Format::RG32Uint:
                return DXGI_FORMAT_R32G32_UINT;
            case Format::RG32Sint:
                return DXGI_FORMAT_R32G32_SINT;
            case Format::RG32Sfloat:
                return DXGI_FORMAT_R32G32_FLOAT;
            case Format::RGB32Uint:
                return DXGI_FORMAT_R32G32B32_UINT;
            case Format::RGB32Sint:
                return DXGI_FORMAT_R32G32B32_SINT;
            case Format::RGB32Sfloat:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case Format::RGBA32Uint:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            case Format::RGBA32Sint:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            case Format::RGBA32Sfloat:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case Format::D16Unorm:
                return DXGI_FORMAT_D16_UNORM;
            case Format::D16UnormS8Uint:
                return DXGI_FORMAT_UNKNOWN; // No DXGI_FORMAT for D16S8
            case Format::D24UnormS8Uint:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Format::D32Sfloat:
                return DXGI_FORMAT_D32_FLOAT;
            case Format::D32SfloatS8Uint:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case Format::S8Uint:
                return DXGI_FORMAT_UNKNOWN; // No DXGI_FORMAT for S8 only
            case Format::BC1RGBUnormBlock:
            case Format::BC1RGBAUnormBlock:
                return DXGI_FORMAT_BC1_UNORM;
            case Format::BC1RGBSrgbBlock:
            case Format::BC1RGBASrgbBlock:
                return DXGI_FORMAT_BC1_UNORM_SRGB;
            case Format::BC2UnormBlock:
                return DXGI_FORMAT_BC2_UNORM;
            case Format::BC2SrgbBlock:
                return DXGI_FORMAT_BC2_UNORM_SRGB;
            case Format::BC3UnormBlock:
                return DXGI_FORMAT_BC3_UNORM;
            case Format::BC3SrgbBlock:
                return DXGI_FORMAT_BC3_UNORM_SRGB;
            case Format::BC4UnormBlock:
                return DXGI_FORMAT_BC4_UNORM;
            case Format::BC4SnormBlock:
                return DXGI_FORMAT_BC4_SNORM;
            case Format::BC5UnormBlock:
                return DXGI_FORMAT_BC5_UNORM;
            case Format::BC5SnormBlock:
                return DXGI_FORMAT_BC5_SNORM;
            case Format::BC6HUfloatBlock:
                return DXGI_FORMAT_BC6H_UF16;
            case Format::BC6HSfloatBlock:
                return DXGI_FORMAT_BC6H_SF16;
            case Format::BC7UnormBlock:
                return DXGI_FORMAT_BC7_UNORM;
            case Format::BC7SrgbBlock:
                return DXGI_FORMAT_BC7_UNORM_SRGB;
            case Format::E5BGR9Ufloat:
                return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case Format::B10GR11Ufloat:
                return DXGI_FORMAT_R11G11B10_FLOAT;
            default:
                return DXGI_FORMAT_UNKNOWN;
            }
        }

        // Sampler Address Mode
        PYRO_FORCEINLINE static constexpr D3D12_TEXTURE_ADDRESS_MODE ToD3D12SamplerAddressMode(SamplerAddressMode mode) {
            switch (mode) {
            case SamplerAddressMode::Repeat:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode::MirroredRepeat:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode::ClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode::ClampToBorder:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode::MirrorClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
        }

        // Border Color
        PYRO_FORCEINLINE static FLOAT* ToD3D12BorderColor(BorderColor color) {
            static FLOAT transparentBlack[] = { 0.f, 0.f, 0.f, 0.f };
            static FLOAT opaqueBlack[] = { 0.f, 0.f, 0.f, 1.f };
            static FLOAT opaqueWhite[] = { 1.f, 1.f, 1.f, 1.f };

            switch (color) {
            case BorderColor::TransparentBlackFloat:
            case BorderColor::TransparentBlackInt:
                return transparentBlack;
            case BorderColor::OpaqueBlackFloat:
            case BorderColor::OpaqueBlackInt:
                return opaqueBlack;
            case BorderColor::OpaqueWhiteFloat:
            case BorderColor::OpaqueWhiteInt:
                return opaqueWhite;
            default:
                return opaqueBlack;
            }
        }

        // CompareOp
        PYRO_FORCEINLINE static constexpr D3D12_COMPARISON_FUNC ToD3D12CompareOp(CompareOp op) {
            switch (op) {
            case CompareOp::Never:
                return D3D12_COMPARISON_FUNC_NEVER;
            case CompareOp::Less:
                return D3D12_COMPARISON_FUNC_LESS;
            case CompareOp::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;
            case CompareOp::LessOrEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case CompareOp::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;
            case CompareOp::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case CompareOp::GreaterOrEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case CompareOp::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;
            default:
                return D3D12_COMPARISON_FUNC_ALWAYS;
            }
        }

        // BlendFactor
        PYRO_FORCEINLINE static constexpr D3D12_BLEND ToD3D12BlendFactor(BlendFactor factor) {
            switch (factor) {
            case BlendFactor::Zero:
                return D3D12_BLEND_ZERO;
            case BlendFactor::One:
                return D3D12_BLEND_ONE;
            case BlendFactor::SrcColor:
                return D3D12_BLEND_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return D3D12_BLEND_INV_SRC_COLOR;
            case BlendFactor::DstColor:
                return D3D12_BLEND_DEST_COLOR;
            case BlendFactor::OneMinusDstColor:
                return D3D12_BLEND_INV_DEST_COLOR;
            case BlendFactor::SrcAlpha:
                return D3D12_BLEND_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return D3D12_BLEND_INV_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return D3D12_BLEND_DEST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return D3D12_BLEND_INV_DEST_ALPHA;
            case BlendFactor::ConstantColor:
                return D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::OneMinusConstantColor:
                return D3D12_BLEND_INV_BLEND_FACTOR;
            case BlendFactor::ConstantAlpha:
                return D3D12_BLEND_BLEND_FACTOR;
            case BlendFactor::OneMinusConstantAlpha:
                return D3D12_BLEND_INV_BLEND_FACTOR;
            case BlendFactor::SrcAlphaSaturate:
                return D3D12_BLEND_SRC_ALPHA_SAT;
            case BlendFactor::Src1Color:
                return D3D12_BLEND_SRC1_COLOR;
            case BlendFactor::OneMinusSrc1Color:
                return D3D12_BLEND_INV_SRC1_COLOR;
            case BlendFactor::Src1Alpha:
                return D3D12_BLEND_SRC1_ALPHA;
            case BlendFactor::OneMinusSrc1Alpha:
                return D3D12_BLEND_INV_SRC1_ALPHA;
            default:
                return D3D12_BLEND_ONE;
            }
        }

        // BlendOp
        PYRO_FORCEINLINE static constexpr D3D12_BLEND_OP ToD3D12BlendOp(BlendOp op) {
            switch (op) {
            case BlendOp::Add:
                return D3D12_BLEND_OP_ADD;
            case BlendOp::Subtract:
                return D3D12_BLEND_OP_SUBTRACT;
            case BlendOp::ReverseSubtract:
                return D3D12_BLEND_OP_REV_SUBTRACT;
            case BlendOp::Min:
                return D3D12_BLEND_OP_MIN;
            case BlendOp::Max:
                return D3D12_BLEND_OP_MAX;
            default:
                return D3D12_BLEND_OP_ADD;
            }
        }

        // PrimitiveTopology
        PYRO_FORCEINLINE static constexpr D3D_PRIMITIVE_TOPOLOGY ToD3D12PrimitiveTopology(PrimitiveTopology topology, u32 numControlPoints) {
            switch (topology) {
            case PrimitiveTopology::PointList:
                return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopology::LineList:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopology::LineStrip:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopology::TriangleList:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveTopology::TriangleFan:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
            case PrimitiveTopology::LineListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            case PrimitiveTopology::LineStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            case PrimitiveTopology::TriangleListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case PrimitiveTopology::TriangleStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            case PrimitiveTopology::PatchList:
                return static_cast<D3D_PRIMITIVE_TOPOLOGY>(static_cast<u32>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) + (numControlPoints - 1)); // DX12 needs explicit patch size
            default:
                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        // PrimitiveTopology
        PYRO_FORCEINLINE static constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12PrimitiveTopologyType(PrimitiveTopology topology) {
            switch (topology) {
            case PrimitiveTopology::PointList:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case PrimitiveTopology::LineList:
            case PrimitiveTopology::LineStrip:
            case PrimitiveTopology::LineListWithAdjacency:
            case PrimitiveTopology::LineStripWithAdjacency:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopology::TriangleList:
            case PrimitiveTopology::TriangleStrip:
            case PrimitiveTopology::TriangleListWithAdjacency:
            case PrimitiveTopology::TriangleStripWithAdjacency:
            case PrimitiveTopology::TriangleFan:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PrimitiveTopology::PatchList:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            default:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
            }
        }

        // FaceCull
        PYRO_FORCEINLINE static constexpr D3D12_CULL_MODE ToD3D12CullMode(FaceCull mode) {
            switch (mode) {
            case FaceCull::None:
                return D3D12_CULL_MODE_NONE;
            case FaceCull::Front:
                return D3D12_CULL_MODE_FRONT;
            case FaceCull::Back:
                return D3D12_CULL_MODE_BACK;
            default:
                return D3D12_CULL_MODE_NONE;
            }
        }

        // StencilOp
        PYRO_FORCEINLINE static constexpr D3D12_STENCIL_OP ToD3D12StencilOp(StencilOp op) {
            switch (op) {
            case StencilOp::Keep:
                return D3D12_STENCIL_OP_KEEP;
            case StencilOp::Zero:
                return D3D12_STENCIL_OP_ZERO;
            case StencilOp::Replace:
                return D3D12_STENCIL_OP_REPLACE;
            case StencilOp::IncrementClamp:
                return D3D12_STENCIL_OP_INCR_SAT;
            case StencilOp::DecrementClamp:
                return D3D12_STENCIL_OP_DECR_SAT;
            case StencilOp::Invert:
                return D3D12_STENCIL_OP_INVERT;
            case StencilOp::IncrementWrap:
                return D3D12_STENCIL_OP_INCR;
            case StencilOp::DecrementWrap:
                return D3D12_STENCIL_OP_DECR;
            default:
                return D3D12_STENCIL_OP_KEEP;
            }
        }
        PYRO_FORCEINLINE static constexpr D3D12_RESOURCE_STATES ToD3D12BufferResourceState(BufferLayout layout) {
            switch (layout) {
            case BufferLayout::Undefined:
                return D3D12_RESOURCE_STATE_COMMON;
            case BufferLayout::UnorderedAccess:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case BufferLayout::ReadOnly:
                return D3D12_RESOURCE_STATE_GENERIC_READ;
            case BufferLayout::TransferSrc:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case BufferLayout::TransferDst:
                return D3D12_RESOURCE_STATE_COPY_DEST;
            default:
                return D3D12_RESOURCE_STATE_COMMON;
            }
        }

        PYRO_FORCEINLINE static constexpr D3D12_RESOURCE_STATES ToD3D12ImageResourceState(ImageLayout layout) {
            switch (layout) {
            case ImageLayout::Undefined:
                return D3D12_RESOURCE_STATE_COMMON;
            case ImageLayout::UnorderedAccess:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case ImageLayout::ReadOnly:
                return D3D12_RESOURCE_STATE_GENERIC_READ;
            case ImageLayout::RenderTarget:
                return D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case ImageLayout::RenderTargetReadOnly:
                return D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_DEPTH_READ;
            case ImageLayout::TransferSrc:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case ImageLayout::TransferDst:
                return D3D12_RESOURCE_STATE_COPY_DEST;
            case ImageLayout::BlitSrc:
                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            case ImageLayout::BlitDst:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;
            case ImageLayout::PresentSrc:
                return D3D12_RESOURCE_STATE_PRESENT;
            default:
                return D3D12_RESOURCE_STATE_COMMON;
            }
        }

        PYRO_FORCEINLINE static constexpr D3D12_RECT ToD3D12Rect(const Rect2D& rect) {
            return {
                .left = static_cast<LONG>(rect.x),
                .top = static_cast<LONG>(rect.y),
                .right = static_cast<LONG>(rect.x + rect.width),
                .bottom = static_cast<LONG>(rect.y + +rect.height),
            };
        }
        PYRO_FORCEINLINE static constexpr D3D12_BOX ToD3D12Box(const Box3D& box) {
            return {
                .left = static_cast<UINT>(box.x),
                .top = static_cast<UINT>(box.y),
                .front = static_cast<UINT>(box.z),
                .right = static_cast<UINT>(box.x + box.width),
                .bottom = static_cast<UINT>(box.y + box.height),
                .back = static_cast<UINT>(box.z + box.depth),
            };
        }


    } // namespace RHIDX12
} // namespace PyroshockStudios