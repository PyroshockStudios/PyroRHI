#pragma once
#include "ICommandBuffer.hpp"
#include "ICommandQueue.hpp"
#include "IDevice.hpp"
#include "IFence.hpp"
#include "ISwapChain.hpp"
#include "Types.hpp"
#include <EASTL/bit.h>
#include <cinttypes>

namespace PyroshockStudios {
    inline namespace RHI {
        namespace internal {
            static eastl::string ToHexImpl(const void* ptr) {
                char buf[32];
                sprintf(buf, "0x%p", ptr);
                return buf;
            }
            static eastl::string ToHexU32Impl(u32 value) {
                char buf[32];
                sprintf(buf, "0x%X", value);
                return buf;
            }

            // Helper for indentation
            static eastl::string IndentImpl(usize indentation) {
                return eastl::string(indentation, ' ');
            }
        } // namespace internal
#define ToHex(x) internal::ToHexImpl(eastl::bit_cast<const void*>(x))
#define ToHexU32(x) internal::ToHexU32Impl(x)
#define Indent(x) internal::IndentImpl(x)


// Helper macro to reduce boilerplate
#define ENUM_TO_STRING_CASE(EnumType, Value) \
    case EnumType::Value:                    \
        return #Value;

        // Format
        inline const char* EnumToString(Format value) {
            switch (value) {
                ENUM_TO_STRING_CASE(Format, Inherit)
                ENUM_TO_STRING_CASE(Format, Undefined)
                ENUM_TO_STRING_CASE(Format, BGRA4Unorm)
                ENUM_TO_STRING_CASE(Format, BGR565Unorm)
                ENUM_TO_STRING_CASE(Format, BGR5A1Unorm)
                ENUM_TO_STRING_CASE(Format, R8Unorm)
                ENUM_TO_STRING_CASE(Format, R8Snorm)
                ENUM_TO_STRING_CASE(Format, R8Uint)
                ENUM_TO_STRING_CASE(Format, R8Sint)
                ENUM_TO_STRING_CASE(Format, R8Srgb)
                ENUM_TO_STRING_CASE(Format, RG8Unorm)
                ENUM_TO_STRING_CASE(Format, RG8Snorm)
                ENUM_TO_STRING_CASE(Format, RG8Uint)
                ENUM_TO_STRING_CASE(Format, RG8Sint)
                ENUM_TO_STRING_CASE(Format, RG8Srgb)
                ENUM_TO_STRING_CASE(Format, RGB8Unorm)
                ENUM_TO_STRING_CASE(Format, RGB8Snorm)
                ENUM_TO_STRING_CASE(Format, RGB8Uint)
                ENUM_TO_STRING_CASE(Format, RGB8Sint)
                ENUM_TO_STRING_CASE(Format, RGB8Srgb)
                ENUM_TO_STRING_CASE(Format, BGR8Unorm)
                ENUM_TO_STRING_CASE(Format, BGR8Snorm)
                ENUM_TO_STRING_CASE(Format, BGR8Uint)
                ENUM_TO_STRING_CASE(Format, BGR8Sint)
                ENUM_TO_STRING_CASE(Format, BGR8Srgb)
                ENUM_TO_STRING_CASE(Format, RGBA8Unorm)
                ENUM_TO_STRING_CASE(Format, RGBA8Snorm)
                ENUM_TO_STRING_CASE(Format, RGBA8Uint)
                ENUM_TO_STRING_CASE(Format, RGBA8Sint)
                ENUM_TO_STRING_CASE(Format, RGBA8Srgb)
                ENUM_TO_STRING_CASE(Format, BGRA8Unorm)
                ENUM_TO_STRING_CASE(Format, BGRA8Snorm)
                ENUM_TO_STRING_CASE(Format, BGRA8Uint)
                ENUM_TO_STRING_CASE(Format, BGRA8Sint)
                ENUM_TO_STRING_CASE(Format, BGRA8Srgb)
                ENUM_TO_STRING_CASE(Format, A2RGB10Unorm)
                ENUM_TO_STRING_CASE(Format, A2RGB10Snorm)
                ENUM_TO_STRING_CASE(Format, A2RGB10Uint)
                ENUM_TO_STRING_CASE(Format, A2RGB10Sint)
                ENUM_TO_STRING_CASE(Format, R16Unorm)
                ENUM_TO_STRING_CASE(Format, R16Snorm)
                ENUM_TO_STRING_CASE(Format, R16Uint)
                ENUM_TO_STRING_CASE(Format, R16Sint)
                ENUM_TO_STRING_CASE(Format, R16Sfloat)
                ENUM_TO_STRING_CASE(Format, RG16Unorm)
                ENUM_TO_STRING_CASE(Format, RG16Snorm)
                ENUM_TO_STRING_CASE(Format, RG16Uint)
                ENUM_TO_STRING_CASE(Format, RG16Sint)
                ENUM_TO_STRING_CASE(Format, RG16Sfloat)
                ENUM_TO_STRING_CASE(Format, RGB16Unorm)
                ENUM_TO_STRING_CASE(Format, RGB16Snorm)
                ENUM_TO_STRING_CASE(Format, RGB16Uint)
                ENUM_TO_STRING_CASE(Format, RGB16Sint)
                ENUM_TO_STRING_CASE(Format, RGB16Sfloat)
                ENUM_TO_STRING_CASE(Format, RGBA16Unorm)
                ENUM_TO_STRING_CASE(Format, RGBA16Snorm)
                ENUM_TO_STRING_CASE(Format, RGBA16Uint)
                ENUM_TO_STRING_CASE(Format, RGBA16Sint)
                ENUM_TO_STRING_CASE(Format, RGBA16Sfloat)
                ENUM_TO_STRING_CASE(Format, R32Uint)
                ENUM_TO_STRING_CASE(Format, R32Sint)
                ENUM_TO_STRING_CASE(Format, R32Sfloat)
                ENUM_TO_STRING_CASE(Format, RG32Uint)
                ENUM_TO_STRING_CASE(Format, RG32Sint)
                ENUM_TO_STRING_CASE(Format, RG32Sfloat)
                ENUM_TO_STRING_CASE(Format, RGB32Uint)
                ENUM_TO_STRING_CASE(Format, RGB32Sint)
                ENUM_TO_STRING_CASE(Format, RGB32Sfloat)
                ENUM_TO_STRING_CASE(Format, RGBA32Uint)
                ENUM_TO_STRING_CASE(Format, RGBA32Sint)
                ENUM_TO_STRING_CASE(Format, RGBA32Sfloat)
                ENUM_TO_STRING_CASE(Format, B10GR11Ufloat)
                ENUM_TO_STRING_CASE(Format, E5BGR9Ufloat)
                ENUM_TO_STRING_CASE(Format, D16Unorm)
                ENUM_TO_STRING_CASE(Format, X8D24Unorm)
                ENUM_TO_STRING_CASE(Format, D32Sfloat)
                ENUM_TO_STRING_CASE(Format, S8Uint)
                ENUM_TO_STRING_CASE(Format, D16UnormS8Uint)
                ENUM_TO_STRING_CASE(Format, D24UnormS8Uint)
                ENUM_TO_STRING_CASE(Format, D32SfloatS8Uint)
                ENUM_TO_STRING_CASE(Format, BC1RGBUnormBlock)
                ENUM_TO_STRING_CASE(Format, BC1RGBSrgbBlock)
                ENUM_TO_STRING_CASE(Format, BC1RGBAUnormBlock)
                ENUM_TO_STRING_CASE(Format, BC1RGBASrgbBlock)
                ENUM_TO_STRING_CASE(Format, BC2UnormBlock)
                ENUM_TO_STRING_CASE(Format, BC2SrgbBlock)
                ENUM_TO_STRING_CASE(Format, BC3UnormBlock)
                ENUM_TO_STRING_CASE(Format, BC3SrgbBlock)
                ENUM_TO_STRING_CASE(Format, BC4UnormBlock)
                ENUM_TO_STRING_CASE(Format, BC4SnormBlock)
                ENUM_TO_STRING_CASE(Format, BC5UnormBlock)
                ENUM_TO_STRING_CASE(Format, BC5SnormBlock)
                ENUM_TO_STRING_CASE(Format, BC6HUfloatBlock)
                ENUM_TO_STRING_CASE(Format, BC6HSfloatBlock)
                ENUM_TO_STRING_CASE(Format, BC7UnormBlock)
                ENUM_TO_STRING_CASE(Format, BC7SrgbBlock)
            default:
                return "Unknown";
            }
        }

        // Example for smaller enums
        inline const char* EnumToString(SamplerAddressMode value) {
            switch (value) {
                ENUM_TO_STRING_CASE(SamplerAddressMode, Repeat)
                ENUM_TO_STRING_CASE(SamplerAddressMode, MirroredRepeat)
                ENUM_TO_STRING_CASE(SamplerAddressMode, ClampToEdge)
                ENUM_TO_STRING_CASE(SamplerAddressMode, ClampToBorder)
                ENUM_TO_STRING_CASE(SamplerAddressMode, MirrorClampToEdge)
            default:
                return "Unknown";
            }
        }

        inline const char* EnumToString(BorderColor value) {
            switch (value) {
                ENUM_TO_STRING_CASE(BorderColor, TransparentBlackFloat)
                ENUM_TO_STRING_CASE(BorderColor, TransparentBlackInt)
                ENUM_TO_STRING_CASE(BorderColor, OpaqueBlackFloat)
                ENUM_TO_STRING_CASE(BorderColor, OpaqueBlackInt)
                ENUM_TO_STRING_CASE(BorderColor, OpaqueWhiteFloat)
                ENUM_TO_STRING_CASE(BorderColor, OpaqueWhiteInt)
            default:
                return "Unknown";
            }
        }


        inline const char* EnumToString(CompareOp value) {
            switch (value) {
                ENUM_TO_STRING_CASE(CompareOp, Never)
                ENUM_TO_STRING_CASE(CompareOp, Less)
                ENUM_TO_STRING_CASE(CompareOp, Equal)
                ENUM_TO_STRING_CASE(CompareOp, LessOrEqual)
                ENUM_TO_STRING_CASE(CompareOp, Greater)
                ENUM_TO_STRING_CASE(CompareOp, NotEqual)
                ENUM_TO_STRING_CASE(CompareOp, GreaterOrEqual)
                ENUM_TO_STRING_CASE(CompareOp, Always)
            default:
                return "Unknown";
            }
        }

        inline const char* EnumToString(BlendFactor value) {
            switch (value) {
                ENUM_TO_STRING_CASE(BlendFactor, Zero)
                ENUM_TO_STRING_CASE(BlendFactor, One)
                ENUM_TO_STRING_CASE(BlendFactor, SrcColor)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusSrcColor)
                ENUM_TO_STRING_CASE(BlendFactor, DstColor)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusDstColor)
                ENUM_TO_STRING_CASE(BlendFactor, SrcAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusSrcAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, DstAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusDstAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, ConstantColor)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusConstantColor)
                ENUM_TO_STRING_CASE(BlendFactor, ConstantAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusConstantAlpha)
                ENUM_TO_STRING_CASE(BlendFactor, SrcAlphaSaturate)
                ENUM_TO_STRING_CASE(BlendFactor, Src1Color)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusSrc1Color)
                ENUM_TO_STRING_CASE(BlendFactor, Src1Alpha)
                ENUM_TO_STRING_CASE(BlendFactor, OneMinusSrc1Alpha)
            }
        }

        inline const char* EnumToString(BlendOp value) {
            switch (value) {
                ENUM_TO_STRING_CASE(BlendOp, Add)
                ENUM_TO_STRING_CASE(BlendOp, Subtract)
                ENUM_TO_STRING_CASE(BlendOp, ReverseSubtract)
                ENUM_TO_STRING_CASE(BlendOp, Min)
                ENUM_TO_STRING_CASE(BlendOp, Max)
            }
        }

        inline const char* EnumToString(TesselationDomainOrigin value) {
            switch (value) {
                ENUM_TO_STRING_CASE(TesselationDomainOrigin, LowerLeft)
                ENUM_TO_STRING_CASE(TesselationDomainOrigin, UpperLeft)
            }
        }

        inline const char* EnumToString(PrimitiveTopology value) {
            switch (value) {
                ENUM_TO_STRING_CASE(PrimitiveTopology, PointList)
                ENUM_TO_STRING_CASE(PrimitiveTopology, LineList)
                ENUM_TO_STRING_CASE(PrimitiveTopology, LineStrip)
                ENUM_TO_STRING_CASE(PrimitiveTopology, TriangleList)
                ENUM_TO_STRING_CASE(PrimitiveTopology, TriangleStrip)
                ENUM_TO_STRING_CASE(PrimitiveTopology, TriangleFan)
                ENUM_TO_STRING_CASE(PrimitiveTopology, LineListWithAdjacency)
                ENUM_TO_STRING_CASE(PrimitiveTopology, LineStripWithAdjacency)
                ENUM_TO_STRING_CASE(PrimitiveTopology, TriangleListWithAdjacency)
                ENUM_TO_STRING_CASE(PrimitiveTopology, TriangleStripWithAdjacency)
                ENUM_TO_STRING_CASE(PrimitiveTopology, PatchList)
            }
        }

        inline const char* EnumToString(PolygonMode value) {
            switch (value) {
                ENUM_TO_STRING_CASE(PolygonMode, Triangle)
                ENUM_TO_STRING_CASE(PolygonMode, Line)
                ENUM_TO_STRING_CASE(PolygonMode, Point)
            }
        }

        inline const char* EnumToString(LineMode value) {
            switch (value) {
                ENUM_TO_STRING_CASE(LineMode, Normal)
                ENUM_TO_STRING_CASE(LineMode, Smooth)
            }
        }

        inline const char* EnumToString(WindingOrder value) {
            switch (value) {
                ENUM_TO_STRING_CASE(WindingOrder, CounterClockwise)
                ENUM_TO_STRING_CASE(WindingOrder, Clockwise)
            }
        }

        inline const char* EnumToString(FaceCull value) {
            switch (value) {
                ENUM_TO_STRING_CASE(FaceCull, None)
                ENUM_TO_STRING_CASE(FaceCull, Front)
                ENUM_TO_STRING_CASE(FaceCull, Back)
            }
        }

        inline const char* EnumToString(StencilOp value) {
            switch (value) {
                ENUM_TO_STRING_CASE(StencilOp, Keep)
                ENUM_TO_STRING_CASE(StencilOp, Zero)
                ENUM_TO_STRING_CASE(StencilOp, Replace)
                ENUM_TO_STRING_CASE(StencilOp, IncrementClamp)
                ENUM_TO_STRING_CASE(StencilOp, DecrementClamp)
                ENUM_TO_STRING_CASE(StencilOp, Invert)
                ENUM_TO_STRING_CASE(StencilOp, IncrementWrap)
                ENUM_TO_STRING_CASE(StencilOp, DecrementWrap)
            }
        }

        inline const char* EnumToString(RasterizationSamples value) {
            switch (value) {
                ENUM_TO_STRING_CASE(RasterizationSamples, e1)
                ENUM_TO_STRING_CASE(RasterizationSamples, e2)
                ENUM_TO_STRING_CASE(RasterizationSamples, e4)
                ENUM_TO_STRING_CASE(RasterizationSamples, e8)
                ENUM_TO_STRING_CASE(RasterizationSamples, e16)
                ENUM_TO_STRING_CASE(RasterizationSamples, e32)
                ENUM_TO_STRING_CASE(RasterizationSamples, e64)
            }
        }

        inline const char* EnumToString(PipelineBindPoint value) {
            switch (value) {
                ENUM_TO_STRING_CASE(PipelineBindPoint, None)
                ENUM_TO_STRING_CASE(PipelineBindPoint, Graphics)
                ENUM_TO_STRING_CASE(PipelineBindPoint, Compute)
                ENUM_TO_STRING_CASE(PipelineBindPoint, RayTracing)
            }
        }

        inline const char* EnumToString(MemoryAllocationDomain value) {
            switch (value) {
                ENUM_TO_STRING_CASE(MemoryAllocationDomain, DeviceLocal)
                ENUM_TO_STRING_CASE(MemoryAllocationDomain, HostStaging)
                ENUM_TO_STRING_CASE(MemoryAllocationDomain, HostRandomWrite)
                ENUM_TO_STRING_CASE(MemoryAllocationDomain, HostReadback)
            }
        }

        inline const char* EnumToString(ColorSpace value) {
            switch (value) {
                ENUM_TO_STRING_CASE(ColorSpace, SrgbNonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, DisplayP3Nonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, ExtendedSrgbLinear)
                ENUM_TO_STRING_CASE(ColorSpace, DisplayP3Linear)
                ENUM_TO_STRING_CASE(ColorSpace, DciP3Nonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, Bt709Linear)
                ENUM_TO_STRING_CASE(ColorSpace, Bt709Nonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, Bt2020Linear)
                ENUM_TO_STRING_CASE(ColorSpace, Hdr10St2084)
                ENUM_TO_STRING_CASE(ColorSpace, Dolbyvision)
                ENUM_TO_STRING_CASE(ColorSpace, Hdr10Hlg)
                ENUM_TO_STRING_CASE(ColorSpace, AdobergbLinear)
                ENUM_TO_STRING_CASE(ColorSpace, AdobergbNonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, PassThrough)
                ENUM_TO_STRING_CASE(ColorSpace, ExtendedSrgbNonlinear)
                ENUM_TO_STRING_CASE(ColorSpace, DisplayNativeAmd)
            }
        }

        inline const char* EnumToString(ImageLayout value) {
            switch (value) {
                ENUM_TO_STRING_CASE(ImageLayout, Identity)
                ENUM_TO_STRING_CASE(ImageLayout, Undefined)
                ENUM_TO_STRING_CASE(ImageLayout, UnorderedAccess)
                ENUM_TO_STRING_CASE(ImageLayout, ReadOnly)
                ENUM_TO_STRING_CASE(ImageLayout, RenderTarget)
                ENUM_TO_STRING_CASE(ImageLayout, RenderTargetReadOnly)
                ENUM_TO_STRING_CASE(ImageLayout, TransferSrc)
                ENUM_TO_STRING_CASE(ImageLayout, TransferDst)
                ENUM_TO_STRING_CASE(ImageLayout, BlitSrc)
                ENUM_TO_STRING_CASE(ImageLayout, BlitDst)
                ENUM_TO_STRING_CASE(ImageLayout, PresentSrc)
            }
        }

        inline const char* EnumToString(BufferLayout value) {
            switch (value) {
                ENUM_TO_STRING_CASE(BufferLayout, Identity)
                ENUM_TO_STRING_CASE(BufferLayout, Undefined)
                ENUM_TO_STRING_CASE(BufferLayout, UnorderedAccess)
                ENUM_TO_STRING_CASE(BufferLayout, ReadOnly)
                ENUM_TO_STRING_CASE(BufferLayout, TransferSrc)
                ENUM_TO_STRING_CASE(BufferLayout, TransferDst)
            }
        }

        inline const char* EnumToString(Filter value) {
            switch (value) {
                ENUM_TO_STRING_CASE(Filter, Nearest)
                ENUM_TO_STRING_CASE(Filter, Linear)
            }
        }

        inline const char* EnumToString(ReductionMode value) {
            switch (value) {
                ENUM_TO_STRING_CASE(ReductionMode, WeightedAverage)
                ENUM_TO_STRING_CASE(ReductionMode, Min)
                ENUM_TO_STRING_CASE(ReductionMode, Max)
            }
        }

        inline const char* EnumToString(IndexType value) {
            switch (value) {
                ENUM_TO_STRING_CASE(IndexType, Uint16)
                ENUM_TO_STRING_CASE(IndexType, Uint32)
                ENUM_TO_STRING_CASE(IndexType, Uint8)
                ENUM_TO_STRING_CASE(IndexType, None)
            }
        }

        inline const char* EnumToString(AttachmentLoadOp value) {
            switch (value) {
                ENUM_TO_STRING_CASE(AttachmentLoadOp, Load)
                ENUM_TO_STRING_CASE(AttachmentLoadOp, Clear)
                ENUM_TO_STRING_CASE(AttachmentLoadOp, DontCare)
            }
        }
        inline const char* EnumToString(AttachmentStoreOp value) {
            switch (value) {
                ENUM_TO_STRING_CASE(AttachmentStoreOp, Store)
                ENUM_TO_STRING_CASE(AttachmentStoreOp, DontCare)
            }
        }

        struct DepthStencilClearValue {
    f32 depth = {};
    u32 stencil = {};

    PYRO_NODISCARD eastl::string ToString(usize) const {
        return eastl::string::format("DepthStencilClearValue{ depth=%.3f, stencil=%u }", depth, stencil);
    }
};

union ColorClearValue {
    eastl::array<f32, 4> float32 = {};
    eastl::array<i32, 4> int32;
    eastl::array<u32, 4> uint32;

    PYRO_NODISCARD eastl::string ToString(usize) const {
        return eastl::string::format("ColorClearValue{ %u, %u, %u, %u }", uint32[0], uint32[1], uint32[2], uint32[3]);
    }
};

    eastl::string Offset2D::ToString(usize indentation) const {
        return eastl::string::format("Offset2D{ x=%d, y=%d }", x, y);
    }

    eastl::string Offset3D::ToString(usize indentation) const {
        return eastl::string::format("Offset3D{ x=%d, y=%d, z=%d }", x, y, z);
    }

    PYRO_NODISCARD eastl::string Extent2D::ToString(usize indentation) const {
        return eastl::string::format("Extent2D{ %u, %u }", width, height);
    }
    PYRO_NODISCARD eastl::string Extent3D::ToString(usize indentation) const {
        return eastl::string::format("{ %u, %u, %u }", width, height, depth);
    }

    eastl::string ViewportInfo::ToString(usize indentation) const {
        eastl::string indent(indentation, ' ');
        return eastl::string().sprintf(
            "ViewportInfo{\n%s  x=%.3f,\n%s  y=%.3f,\n%s  width=%.3f,\n%s  height=%.3f,\n%s  minDepth=%.3f,\n%s  maxDepth=%.3f\n%s}",
            indent.c_str(), x,
            indent.c_str(), y,
            indent.c_str(), width,
            indent.c_str(), height,
            indent.c_str(), minDepth,
            indent.c_str(), maxDepth,
            indent.c_str()
        );
    }
    eastl::string Rect2D::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("Rect2D{ x=%u, y=%u, width=%u, height=%u }", x, y, width, height);
    }

    eastl::string Box3D::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("Box3D{ x=%u, y=%u, z=%u, width=%u, height=%u, depth=%u }", x, y, z, width, height, depth);
    }

    eastl::string BufferRegion::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("{ offset=%"PRIu64", size=%zu }", offset, size);
    }

    eastl::string ImageMipArraySlice::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("{ firstMip=%u, mipCount=%u, firstLayer=%u, layerCount=%u }",
                                     baseMipLevel, levelCount, baseArrayLayer, layerCount);
    }
     eastl::string ImageArraySlice::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("{ mip=%u, firstLayer=%u, layerCount=%u }",
                                     mipLevel, baseArrayLayer, layerCount);
    }
    eastl::string ImageSlice::ToString(usize indentation) const {
        return Indent(indentation) + eastl::string().sprintf("{ mip=%u, layer=%u }", mipLevel, arrayLayer);
    }




        eastl::string DeviceInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            static const auto deviceTypeToString = [](DeviceType type) {
                switch (type) {
                case DeviceType::Unknown:
                    return "Unknown";
                case DeviceType::Discrete:
                    return "Discrete";
                case DeviceType::Integrated:
                    return "Integrated";
                case DeviceType::Virtual:
                    return "Virtual";
                case DeviceType::CPU:
                    return "CPU";
                default:
                    return "Invalid";
                }
            };

            eastl::string s;
            s += indent + "DeviceInfo {\n";
            s += indent + "  Name: " + name + "\n";
            s += indent + "  Vendor: " + vendor + "\n";
            s += indent + "  VendorID: " + eastl::to_string(vendorID) + "\n";
            s += indent + "  DeviceID: " + eastl::to_string(deviceID) + "\n";
            s += indent + "  SubsystemID: " + eastl::to_string(subsystemID) + "\n";
            s += indent + "  RevisionID: " + eastl::to_string(revisionID) + "\n";
            s += indent + "  DeviceType: " + deviceTypeToString(deviceType) + "\n";
            s += indent + "  UnifiedMemory: " + (bUnifiedMemory ? "true" : "false") + "\n";
            s += indent + "  Removable: " + (bRemovable ? "true" : "false") + "\n";
            s += indent + "  PrimaryAdapter: " + (bPrimaryAdapter ? "true" : "false") + "\n";
            s += indent + "  Headless: " + (bHeadless ? "true" : "false") + "\n";
            s += indent + "  DriverVersion: " + driverVersion + "\n";
            s += indent + "  APIVersion: " + apiVersion + "\n";
            s += indent + "  DriverDescription: " + driverDescription + "\n";
            s += indent + "  Architecture: " + architecture + "\n";
            s += indent + "  DedicatedVRAM: " + eastl::to_string(dedicatedVideoMemory) + "\n";
            s += indent + "  SharedSystemMemory: " + eastl::to_string(sharedSystemMemory) + "\n";
            s += indent + "  AdapterLUID: " + eastl::to_string(adapterLUIDHigh) + ":" + eastl::to_string(adapterLUIDLow) + "\n";
            s += indent + "}";
            return s;
        }

        eastl::string DeviceFeaturesInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "DeviceFeaturesInfo {\n";
#define BOOL_FIELD(name) \
    s += indent + "  " + eastl::string(#name + 1) + ": " + (name ? "true" : "false") + "\n";
            BOOL_FIELD(bGeometryShaders);
            BOOL_FIELD(bTesselationShaders);
            BOOL_FIELD(bMeshShaders);
            BOOL_FIELD(bTaskShaders);
            BOOL_FIELD(bRayQueries);
            BOOL_FIELD(bRayTracingPipelines);
            BOOL_FIELD(bAccelerationStructureBuild);
            BOOL_FIELD(bBufferDeviceAddress);
            BOOL_FIELD(bBCnTextureCompression);
            BOOL_FIELD(bUint8IndexBuffer);
            BOOL_FIELD(bInt64ShaderOps);
            BOOL_FIELD(bAtomicFloatOps);
            BOOL_FIELD(bWaveOps);
            BOOL_FIELD(bSubgroupQuadOps);
            BOOL_FIELD(bHeadlessSwapChainWindow);
            BOOL_FIELD(bVariableRateShading);
            BOOL_FIELD(bConservativeRasterization);
#undef BOOL_FIELD
            s += indent + "  SupportedShaderModel: " + eastl::to_string(supportedShaderModel) + "\n";
            s += indent + "}";
            return s;
        }

        eastl::string DevicePropertiesInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "DevicePropertiesInfo {\n";
            s += indent + "  MSAA ColorTarget: " + eastl::to_string((u32)msaaSupportColorTarget) + "\n";
            s += indent + "  MSAA DepthStencilTarget: " + eastl::to_string((u32)msaaSupportDepthStencilTarget) + "\n";
            s += indent + "  MSAA ShaderResourceView: " + eastl::to_string((u32)msaaSupportShaderResourceView) + "\n";
            s += indent + "  MSAA UAV: " + eastl::to_string((u32)msaaSupportUnorderedAccessView) + "\n";
            s += indent + "  BufferImageRowAlignment: " + eastl::to_string(bufferImageRowAlignment) + "\n";
            s += indent + "  BufferImageCopyOffsetAlignment: " + eastl::to_string(bufferImageCopyOffsetAlignment) + "\n";
            s += indent + "  MinUniformBufferOffsetAlignment: " + eastl::to_string(minUniformBufferOffsetAlignment) + "\n";
            s += indent + "  MinStorageBufferOffsetAlignment: " + eastl::to_string(minStorageBufferOffsetAlignment) + "\n";
            s += indent + "  QueueCounts: G=" + eastl::to_string(graphicsQueueCount) +
                 " C=" + eastl::to_string(computeQueueCount) +
                 " T=" + eastl::to_string(transferQueueCount) + "\n";
            s += indent + "  DedicatedQueues: Compute=" + eastl::to_string(bHasDedicatedComputeQueue) +
                 " Transfer=" + eastl::to_string(bHasDedicatedTransferQueue) + "\n";
            s += indent + "  TextureLimits: " + eastl::to_string(maxTextureWidth) + "x" +
                 eastl::to_string(maxTextureHeight) + "x" + eastl::to_string(maxTextureDepth) + "\n";
            s += indent + "  MaxTextureArrayLayers: " + eastl::to_string(maxTextureArrayLayers) + "\n";
            s += indent + "  MaxSamplerAnisotropy: " + eastl::to_string(maxSamplerAnisotropy) + "\n";
            s += indent + "  LineWidthRange: " + eastl::to_string(minLineWidth) + " - " + eastl::to_string(maxLineWidth) + "\n";
            s += indent + "}";
            return s;
        }

        eastl::string DeviceStatusInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "DeviceStatusInfo {\n";
            s += indent + "  HostAllocations: " + eastl::to_string(numHostAllocations) + " (" + eastl::to_string(numHostAllocatedBytes) + " bytes)\n";
            s += indent + "  DeviceAllocations: " + eastl::to_string(numDeviceMemoryAllocations) + " (" + eastl::to_string(numDeviceAllocatedBytes) + " bytes)\n";
            s += indent + "  BuffersCreated: " + eastl::to_string(numBufferResourcesCreated) + "\n";
            s += indent + "  ImagesCreated: " + eastl::to_string(numImageResourcesCreated) + "\n";
            s += indent + "  PipelinesCreated: " + eastl::to_string(numPipelineObjectsCreated) + "\n";
            s += indent + "  DescriptorHeapsCreated: " + eastl::to_string(numDescriptorHeapsCreated) + "\n";
            s += indent + "  CommandBuffersCreated: " + eastl::to_string(numCommandBuffersCreated) + "\n";
            s += indent + "  #Alive: Buffers=" + eastl::to_string(numBuffersAlive) +
                 ", Images=" + eastl::to_string(numImagesAlive) +
                 ", Pipelines=" + eastl::to_string(numPipelinesAlive) +
                 ", Samplers=" + eastl::to_string(numSamplersAlive) + "\n";
            s += indent + "  QueueSubmits: " + eastl::to_string(numQueueSubmits) + "\n";
            s += indent + "  AvailableVRAM: " + eastl::to_string(availableVideoMemory) + "\n";
            s += indent + "}";
            return s;
        }

        eastl::string SemaphoreSubmitInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "SemaphoreSubmitInfo { Semaphore=";
            s += (ToHex(semaphore));
            s += ", Stage=" + eastl::to_string(static_cast<u32>(stage)) + " }";
            return s;
        }

        eastl::string FenceSubmitInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "FenceSubmitInfo { Fence=";
            s += (ToHex(fence));
            s += ", Value=" + eastl::to_string(value) + " }";
            return s;
        }

        eastl::string CommandQueueSubmitInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "CommandQueueSubmitInfo {\n";
            s += indent + "  Queue: " + (ToHex(queue)) + "\n";
            s += indent + "  WaitSemaphores: [\n";
            for (const auto& subInfo : waitSemaphores) {
                s += indent + subInfo.ToString(indentation + 4) + "\n";
            }
            s += indent + "]\n";

            s += indent + "  SignalSemaphores: [\n";
            for (const auto& subInfo : signalSemaphores) {
                s += indent + subInfo.ToString(indentation + 4) + "\n";
            }
            s += indent + "]\n";

            s += indent + "  SignalPresentSemaphores: [\n";
            for (const auto& subInfo : signalPresentReadySemaphores) {
                s += indent + subInfo.ToString(indentation + 4) + "\n";
            }
            s += indent + "]\n";

            s += indent + "  SignalFences: [\n";
            for (const auto& subInfo : signalFences) {
                s += indent + subInfo.ToString(indentation + 4) + "\n";
            }
            s += indent + "]\n";
            s += indent + "}";
            return s;
        }

        eastl::string CommandQueuePresentInfo::ToString(usize indentation) const {
            const eastl::string indent = eastl::string(indentation, ' ');
            eastl::string s;
            s += indent + "CommandQueuePresentInfo { Queue=";
            s += indent + (ToHex(queue));
            s += ", WaitSemaphores=" + eastl::to_string(waitSemaphores.size()) + " }";
            return s;
        }

        // ============================ ToString Implementations ============================

        eastl::string CommandBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CommandBufferInfo { name: \"" + name + "\" }";
            return s;
        }

        eastl::string CopyBufferToBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CopyBufferToBufferInfo {\n";
            s += Indent(indentation + 2) + "srcBuffer: " + ToHex(srcBuffer) + "\n";
            s += Indent(indentation + 2) + "dstBuffer: " + ToHex(dstBuffer) + "\n";
            s += Indent(indentation + 2) + "srcOffset: " + eastl::to_string(srcOffset) + "\n";
            s += Indent(indentation + 2) + "dstOffset: " + eastl::to_string(dstOffset) + "\n";
            s += Indent(indentation + 2) + "size: " + eastl::to_string(size) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string CopyBufferToImageInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CopyBufferToImageInfo {\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "bufferOffset: " + bufferOffset + "\n";
            s += Indent(indentation + 2) + "image: " + ToHex(image) + "\n";
            s += Indent(indentation + 2) + "imageSlice: " + imageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "imageOffset: " + imageOffset.ToString() + "\n";
            s += Indent(indentation + 2) + "imageExtent: " + imageExtent.ToString() + "\n";
            s += Indent(indentation + 2) + "rowPitch: " + eastl::to_string(rowPitch) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string CopyImageToBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CopyImageToBufferInfo {\n";
            s += Indent(indentation + 2) + "image: " + ToHex(image) + "\n";
            s += Indent(indentation + 2) + "imageSlice: " + imageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "imageOffset: " + imageOffset.ToString() + "\n";
            s += Indent(indentation + 2) + "imageExtent: " + imageExtent.ToString() + "\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "bufferOffset: " + eastl::to_string(bufferOffset) + "\n";
            s += Indent(indentation + 2) + "rowPitch: " + eastl::to_string(rowPitch) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string CopyImageToImageInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CopyImageToImageInfo {\n";
            s += Indent(indentation + 2) + "srcImage: " + ToHex(srcImage) + "\n";
            s += Indent(indentation + 2) + "dstImage: " + ToHex(dstImage) + "\n";
            s += Indent(indentation + 2) + "srcImageSlice: " + srcImageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "srcOffset: " + srcOffset.ToString() + "\n";
            s += Indent(indentation + 2) + "dstImageSlice: " + dstImageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "dstOffset: " + dstOffset.ToString() + "\n";
            s += Indent(indentation + 2) + "extent: " + extent.ToString() + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string BlitImageToImageInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "BlitImageToImageInfo {\n";
            s += Indent(indentation + 2) + "srcImage: " + ToHex(srcImage) + "\n";
            s += Indent(indentation + 2) + "dstImage: " + ToHex(dstImage) + "\n";
            s += Indent(indentation + 2) + "srcImageSlice: " + srcImageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "srcImageBox: " + srcImageBox.ToString() + "\n";
            s += Indent(indentation + 2) + "dstImageSlice: " + dstImageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "dstImageBox: " + dstImageBox.ToString() + "\n";
            s += Indent(indentation + 2) + "filter: " + EnumToString(filter) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string ClearUnorderedAccessViewInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "ClearUAVInfo {\n";
            s += Indent(indentation + 2) + "view: " + ToHex(view) + "\n";
            s += Indent(indentation + 2) + "clearValue: " + clearValue.ToString() + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string UpdateBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "UpdateBufferInfo {\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "region: " + region.ToString() + "\n";
            s += Indent(indentation + 2) + "data: " + ToHex(data) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string BufferMemoryBarrierInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "BufferMemoryBarrierInfo {\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "region: " + region.ToString() + "\n";
            s += Indent(indentation + 2) + "srcAccess: " + eastl::to_string(static_cast<u64>(srcAccess)) + "\n";
            s += Indent(indentation + 2) + "dstAccess: " + eastl::to_string(static_cast<u64>(dstAccess)) + "\n";
            s += Indent(indentation + 2) + "srcLayout: " + EnumToString(srcLayout) + "\n";
            s += Indent(indentation + 2) + "dstLayout: " + EnumToString(dstLayout) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string ImageMemoryBarrierInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "ImageMemoryBarrierInfo {\n";
            s += Indent(indentation + 2) + "image: " + ToHex(image) + "\n";
            s += Indent(indentation + 2) + "imageSlice: " + imageSlice.ToString() + "\n";
            s += Indent(indentation + 2) + "srcAccess: " + eastl::to_string(static_cast<u64>(srcAccess)) + "\n";
            s += Indent(indentation + 2) + "dstAccess: " + eastl::to_string(static_cast<u64>(dstAccess)) + "\n";
            s += Indent(indentation + 2) + "srcLayout: " + EnumToString(srcLayout) + "\n";
            s += Indent(indentation + 2) + "dstLayout: " + EnumToString(dstLayout) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string InvalidateTimestampQueryInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "InvalidateTimestampQueryInfo {\n";
            s += Indent(indentation + 2) + "queryPool: " + ToHex(queryPool) + "\n";
            s += Indent(indentation + 2) + "firstQuery: " + eastl::to_string(firstQuery) + "\n";
            s += Indent(indentation + 2) + "queryCount: " + eastl::to_string(queryCount) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string WriteTimestampInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "WriteTimestampInfo {\n";
            s += Indent(indentation + 2) + "queryPool: " + ToHex(queryPool) + "\n";
            s += Indent(indentation + 2) + "stage: " + eastl::to_string(static_cast<u64>(stage)) + "\n";
            s += Indent(indentation + 2) + "queryIndex: " + eastl::to_string(queryIndex) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string CommandLabelInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "CommandLabelInfo {\n";
            s += Indent(indentation + 2) + "labelColor: " + ToHexU32(eastl::bit_cast<u32>(labelColor)) + "\n";
            s += Indent(indentation + 2) + "name: \"" + eastl::string(name) + "\"\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string AttachmentResolveInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "AttachmentResolveInfo {\n";
            s += Indent(indentation + 2) + "mode: ";
            switch (mode) {
            case ResolveMode::None:
                s += "None";
                break;
            case ResolveMode::SampleZero:
                s += "SampleZero";
                break;
            case ResolveMode::Average:
                s += "Average";
                break;
            case ResolveMode::Min:
                s += "Min";
                break;
            case ResolveMode::Max:
                s += "Max";
                break;
            }
            s += "\n";
            s += Indent(indentation + 2) + "target: " + ToHex(target) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string ColorAttachmentInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "ColorAttachmentInfo {\n";
            s += Indent(indentation + 2) + "target: " + ToHex(target) + "\n";
            s += Indent(indentation + 2) + "loadOp: " + EnumToString(loadOp) + "\n";
            s += Indent(indentation + 2) + "storeOp: " + EnumToString(storeOp) + "\n";
            s += Indent(indentation + 2) + "clearValue: " + clearValue.ToString(indentation + 2) + "\n";
            if (resolve.has_value())
                s += Indent(indentation + 2) + "resolve: " + resolve->ToString(indentation + 2) + "\n";
            else
                s += Indent(indentation + 2) + "resolve: empty\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string DepthStencilAttachmentInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DepthStencilAttachmentInfo {\n";
            s += Indent(indentation + 2) + "target: " + ToHex(target) + "\n";
            s += Indent(indentation + 2) + "depthLoadOp: " + EnumToString(depthLoadOp) + "\n";
            s += Indent(indentation + 2) + "depthStoreOp: " + EnumToString(depthStoreOp) + "\n";
            s += Indent(indentation + 2) + "stencilLoadOp: " + EnumToString(stencilLoadOp) + "\n";
            s += Indent(indentation + 2) + "stencilStoreOp: " + EnumToString(stencilStoreOp) + "\n";
            s += Indent(indentation + 2) + "clearValue: " + clearValue.ToString(indentation + 2) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string RenderPassBeginInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "RenderPassBeginInfo {\n";
            s += Indent(indentation + 2) + "colorAttachments: [\n";
            for (auto& ca : colorAttachments)
                s += ca.ToString(indentation + 4) + ",\n";
            s += Indent(indentation + 2) + "]\n";
            if (depthStencilAttachment.has_value())
                s += Indent(indentation + 2) + "depthStencilAttachment: " + depthStencilAttachment->ToString(indentation + 2) + "\n";
            else
                s += Indent(indentation + 2) + "depthStencilAttachment: empty\n";
            s += Indent(indentation + 2) + "renderArea: " + renderArea.ToString(indentation + 2) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string PushConstantInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "PushConstantInfo {\n";
            s += Indent(indentation + 2) + "data: [";
            const u32* dwords = reinterpret_cast<const u32*>(data);
            for (u32 i = 0; i < size / 4; ++i) {
                if (i > 0)
                    s += ", ";
                s += ToHexU32(dwords[i]);
            }
            s += "]\n";
            s += Indent(indentation + 2) + "size: " + eastl::to_string(size) + "\n";
            s += Indent(indentation + 2) + "offset: " + eastl::to_string(offset) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string SetUniformBufferViewInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "SetUniformBufferViewInfo {\n";
            s += Indent(indentation + 2) + "slot: " + eastl::to_string(slot) + "\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "region: " + region.ToString(indentation + 2) + "\n";
            s += Indent(indentation + 2) + "bindPoint: " + EnumToString(bindPoint) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string SetUnorderedAccessViewInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "SetUnorderedAccessViewInfo {\n";
            s += Indent(indentation + 2) + "slot: " + eastl::to_string(slot) + "\n";
            s += Indent(indentation + 2) + "view: " + ToHex(view) + "\n";
            s += Indent(indentation + 2) + "bindPoint: " + EnumToString(bindPoint) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string SetVertexBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "SetVertexBufferInfo {\n";
            s += Indent(indentation + 2) + "slot: " + eastl::to_string(slot) + "\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "offset: " + eastl::to_string(offset) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string SetIndexBufferInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "SetIndexBufferInfo {\n";
            s += Indent(indentation + 2) + "buffer: " + ToHex(buffer) + "\n";
            s += Indent(indentation + 2) + "offset: " + eastl::to_string(offset) + "\n";
            s += Indent(indentation + 2) + "indexType: " + EnumToString(indexType) + "\n";
            s += Indent(indentation) + "}";
            return s;
        }

        eastl::string DrawInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DrawInfo { vertexCount: " + eastl::to_string(vertexCount) + ", instanceCount: " + eastl::to_string(instanceCount) + ", firstVertex: " + eastl::to_string(firstVertex) + ", firstInstance: " + eastl::to_string(firstInstance) + " }";
            return s;
        }

        eastl::string DrawIndexedInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DrawIndexedInfo { indexCount: " + eastl::to_string(indexCount) + ", instanceCount: " + eastl::to_string(instanceCount) + ", firstIndex: " + eastl::to_string(firstIndex) + ", vertexOffset: " + eastl::to_string(vertexOffset) + ", firstInstance: " + eastl::to_string(firstInstance) + " }";
            return s;
        }

        eastl::string DrawIndirectInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DrawIndirectInfo { indirectBuffer: " + ToHex(indirectBuffer) + ", indirectBufferOffset: " + eastl::to_string(indirectBufferOffset) + ", drawCount: " + eastl::to_string(drawCount) + ", drawCommandStride: " + eastl::to_string(drawCommandStride) + " }";
            return s;
        }

        eastl::string DrawIndexedIndirectInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DrawIndexedIndirectInfo { indirectBuffer: " + ToHex(indirectBuffer) + ", indirectBufferOffset: " + eastl::to_string(indirectBufferOffset) + ", drawCount: " + eastl::to_string(drawCount) + ", drawCommandStride: " + eastl::to_string(drawCommandStride) + " }";
            return s;
        }

        eastl::string DispatchInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DispatchInfo { x: " +
                 eastl::to_string(x) + ", y: " + eastl::to_string(y) + ", z: " + eastl::to_string(z) + " }";
            return s;
        }

        eastl::string DispatchIndirectInfo::ToString(usize indentation) const {
            eastl::string s;
            s += Indent(indentation) + "DispatchIndirectInfo { indirectBuffer: " + ToHex(indirectBuffer) + ", indirectBufferOffset: " + eastl::to_string(indirectBufferOffset) + " }";
            return s;
        }

#undef ToHex
#undef ToHexU32
#undef Indent
#undef ENUM_TO_STRING_CASE
    } // namespace RHI
} // namespace PyroshockStudios