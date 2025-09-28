#pragma once

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Api/Util.hpp>
#include <cassert>

#ifdef PYRO_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#elif PYRO_PLATFORM_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#elif PYRO_PLATFORM_MACOS
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES 1
#ifdef PYRO_PLATFORM_WINDOWS
// #include <vulkan/vulkan_win32.h>
#else

#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <volk.h>
#ifndef _MSC_VER
#pragma GCC diagnostic push
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wnullability-completeness"
#endif
#endif
#pragma warning(push)
#include <vk_mem_alloc.h>
#pragma warning(pop)
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#ifdef PYRO_PLATFORM_WINDOWS
#undef CreateEvent
#undef CreateSemaphore
#endif
#ifdef PYRO_PLATFORM_LINUX
// stupid X11
#undef None
#undef Always
#endif

namespace PyroshockStudios {
    namespace RHIVulkan {
        constexpr u32 MAX_VK_BINDLESS_BUFFERS = 32768;
        constexpr u32 MAX_VK_BINDLESS_IMAGES = 32768;
        constexpr u32 MAX_VK_BINDLESS_SAMPLERS = 2048;

        void CheckVkResult(VkResult result);

        PYRO_FORCEINLINE static constexpr VkFormat ToVkFormat(Format type) { return static_cast<VkFormat>(type); }
        PYRO_FORCEINLINE static constexpr VkSamplerAddressMode ToVkSamplerAddressMode(SamplerAddressMode type) { return static_cast<VkSamplerAddressMode>(type); }
        PYRO_FORCEINLINE static constexpr VkBorderColor ToVkBorderColor(BorderColor type) { return static_cast<VkBorderColor>(type); }
        PYRO_FORCEINLINE static constexpr VkCompareOp ToVkCompareOp(CompareOp type) { return static_cast<VkCompareOp>(type); }
        PYRO_FORCEINLINE static constexpr VkStencilOp ToVkStencilOp(StencilOp type) { return static_cast<VkStencilOp>(type); }
        PYRO_FORCEINLINE static constexpr VkBlendFactor ToVkBlendFactor(BlendFactor type) { return static_cast<VkBlendFactor>(type); }
        PYRO_FORCEINLINE static constexpr VkBlendOp ToVkBlendOp(BlendOp type) { return static_cast<VkBlendOp>(type); }
        PYRO_FORCEINLINE static constexpr VkTessellationDomainOrigin ToVkTesselationDomainOrigin(TesselationDomainOrigin type) { return static_cast<VkTessellationDomainOrigin>(type); }
        PYRO_FORCEINLINE static constexpr VkPrimitiveTopology ToVkPrimitiveTopology(PrimitiveTopology type) { return static_cast<VkPrimitiveTopology>(type); }
        PYRO_FORCEINLINE static constexpr VkPolygonMode ToVkPolygonMode(PolygonMode type) { return static_cast<VkPolygonMode>(type); }
        PYRO_FORCEINLINE static constexpr VkFrontFace ToVkFrontFaceWinding(WindingOrder type) { return static_cast<VkFrontFace>(type); }
        PYRO_FORCEINLINE static constexpr VkPresentModeKHR ToVkPresentMode(PresentMode type) { return static_cast<VkPresentModeKHR>(type); }
        PYRO_FORCEINLINE static constexpr VkColorSpaceKHR ToVkColorSpace(ColorSpace type) { return static_cast<VkColorSpaceKHR>(type); }
        PYRO_FORCEINLINE static constexpr VkFilter ToVkFilter(Filter type) { return static_cast<VkFilter>(type); }
        PYRO_FORCEINLINE static constexpr VkSamplerReductionMode ToVkReductionMode(ReductionMode type) { return static_cast<VkSamplerReductionMode>(type); }
        PYRO_FORCEINLINE static constexpr VkIndexType ToVkIndexType(IndexType type) { return static_cast<VkIndexType>(type); }
        PYRO_FORCEINLINE static constexpr VkAttachmentLoadOp ToVkAttachmentLoadOp(AttachmentLoadOp type) { return static_cast<VkAttachmentLoadOp>(type); }
        PYRO_FORCEINLINE static constexpr VkAttachmentStoreOp ToVkAttachmentStoreOp(AttachmentStoreOp type) { return static_cast<VkAttachmentStoreOp>(type); }
        PYRO_FORCEINLINE static constexpr VkCullModeFlagBits ToVkFaceCull(FaceCull type) { return static_cast<VkCullModeFlagBits>(type); }
        PYRO_FORCEINLINE static constexpr VkColorComponentFlags ToVkColorComponentFlags(ColorComponentFlags type) { return static_cast<VkColorComponentFlags>(type.data); }
        PYRO_FORCEINLINE static constexpr VkAccessFlags2 ToVkAccessTypeFlags(AccessTypeFlags type) { return static_cast<VkAccessFlags2>(type.data); }
        PYRO_FORCEINLINE static constexpr VkPipelineStageFlags2 ToVkPipelineStageFlags(PipelineStageFlags type) { return static_cast<VkPipelineStageFlags2>(type.data); }

        PYRO_FORCEINLINE static constexpr VkFormatFeatureFlags ToVkFormatFeatureFlags(FormatFeatureFlags type) { return static_cast<VkFormatFeatureFlags>(type.data); }
        PYRO_FORCEINLINE static constexpr VkAttachmentLoadOp ToVkLoadOp(AttachmentLoadOp type) { return static_cast<VkAttachmentLoadOp>(type); }
        PYRO_FORCEINLINE static constexpr VkAttachmentStoreOp ToVkStoreOp(AttachmentStoreOp type) { return static_cast<VkAttachmentStoreOp>(type); }

        PYRO_FORCEINLINE static constexpr VkImageUsageFlags ToVkImageUsageFlags(ImageUsageFlags type, Format format) {
            VkImageUsageFlags retFlags = {};

            if (type & ImageUsageFlagBits::TRANSFER_SRC)
                retFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            if (type & ImageUsageFlagBits::TRANSFER_DST)
                retFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if (type & ImageUsageFlagBits::SHADER_RESOURCE)
                retFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
            if (type & ImageUsageFlagBits::UNORDERED_ACCESS)
                retFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
            if (type & ImageUsageFlagBits::RENDER_TARGET)
                retFlags |= RHIUtil::FormatIsDepthStencil(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (type & ImageUsageFlagBits::TRANSIENT_ATTACHMENT)
                retFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
            if (type & ImageUsageFlagBits::FRAGMENT_DENSITY_MAP)
                retFlags |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
            if (type & ImageUsageFlagBits::FRAGMENT_SHADING_RATE_ATTACHMENT)
                retFlags |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
            if (type & ImageUsageFlagBits::BLIT_SRC)
                retFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            if (type & ImageUsageFlagBits::BLIT_DST)
                retFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            
            return retFlags;
        }

        PYRO_FORCEINLINE static constexpr VkImageLayout ToVkImageLayout(ImageLayout type) {
            switch (type) {
            case ImageLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::UnorderedAccess:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ImageLayout::ReadOnly:
                return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            case ImageLayout::RenderTarget:
                return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            case ImageLayout::RenderTargetReadOnly:
                return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
            case ImageLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::BlitSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::BlitDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::PresentSrc:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            default:
                // error!
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        }

        PYRO_FORCEINLINE static constexpr VkPipelineBindPoint ToVkPipelineBindPoint(PipelineBindPoint type) {
            switch (type) {
            case PipelineBindPoint::Graphics:
                return VK_PIPELINE_BIND_POINT_GRAPHICS;
            case PipelineBindPoint::Compute:
                return VK_PIPELINE_BIND_POINT_COMPUTE;
            case PipelineBindPoint::RayTracing:
                return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            default:
                // error!
                return VK_PIPELINE_BIND_POINT_MAX_ENUM;
            }
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios