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

#include "CommandBuffer.hpp"
#include "CommandQueue.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "RenderTarget.hpp"

#include <RHIVulkan/VkContext.hpp>

#include <libassert/assert.hpp>

namespace PyroshockStudios::RHIVulkan {
    eastl::pair<VkCommandPool, VkCommandBuffer> CommandBufferPool::Get(VulkanDevice* device, VulkanCommandQueue* queue) {
        eastl::pair<VkCommandPool, VkCommandBuffer> pair = {};
        if (poolAndBuffers.empty()) {
            VkCommandPool pool = {};
            VkCommandBuffer buffer = {};
            VkCommandPoolCreateInfo const vk_command_pool_create_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue->GetQueueFamily(),
            };

            vkCreateCommandPool(device->GetVkDevice(), &vk_command_pool_create_info, device->Context()->GetVkAllocator(), &pool);

            VkCommandBufferAllocateInfo const vk_command_buffer_allocate_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkAllocateCommandBuffers(device->GetVkDevice(), &vk_command_buffer_allocate_info, &buffer);
            pair = { pool, buffer };
        } else {
            pair = poolAndBuffers.back();
            poolAndBuffers.pop_back();
        }
        return pair;
    }

    void CommandBufferPool::PutBack(eastl::pair<VkCommandPool, VkCommandBuffer> poolAndBuffer) {
        poolAndBuffers.push_back(poolAndBuffer);
    }

    void CommandBufferPool::Cleanup(VulkanDevice* device, VulkanCommandQueue* queue) {
        for (auto [pool, buffer] : poolAndBuffers) {
            vkDestroyCommandPool(device->GetVkDevice(), pool, device->Context()->GetVkAllocator());
        }
        poolAndBuffers.clear();
    }

    PYRO_FORCEINLINE static constexpr VkResolveModeFlagBits ToVkResolveMode(ResolveMode type) { return static_cast<VkResolveModeFlagBits>(type); }
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VulkanCommandQueue* queue, VkCommandPool pool, VkCommandBuffer buffer, const CommandBufferInfo& info)
        : mDevice(device), mQueue(queue), mCommandPool(pool), mCommandBuffer(buffer) {
        const VkCommandBufferBeginInfo vkCommandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = {},
        };

        if (vkSetDebugUtilsObjectNameEXT && !info.name.empty()) {
            const VkDebugUtilsObjectNameInfoEXT cmdBufferNameInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = reinterpret_cast<uint64_t>(this->mCommandBuffer),
                .pObjectName = info.name.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(device->GetVkDevice(), &cmdBufferNameInfo);
            eastl::string poolname = info.name + " (Command Pool)";
            const VkDebugUtilsObjectNameInfoEXT cmdPoolNameInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
                .objectHandle = reinterpret_cast<uint64_t>(this->mCommandPool),
                .pObjectName = poolname.c_str(),
            };
            vkSetDebugUtilsObjectNameEXT(device->GetVkDevice(), &cmdPoolNameInfo);
        }

        vkBeginCommandBuffer(this->mCommandBuffer, &vkCommandBufferBeginInfo);
    }
    VulkanCommandBuffer::~VulkanCommandBuffer() {
    }

    void VulkanCommandBuffer::CopyBufferToBuffer(const CopyBufferToBufferInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        const VkBufferCopy copyRegion = {
            .srcOffset = info.srcOffset,
            .dstOffset = info.dstOffset,
            .size = info.size
        };
        vkCmdCopyBuffer(mCommandBuffer, mDevice->Slot(info.srcBuffer).vkBuffer, mDevice->Slot(info.dstBuffer).vkBuffer, 1, &copyRegion);
    }

    void VulkanCommandBuffer::CopyBufferToImage(const CopyBufferToImageInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplImageSlot& imageSlot = mDevice->Slot(info.image);

        const VkBufferImageCopy region = {
            .bufferOffset = info.bufferOffset,
            .bufferRowLength = info.rowPitch == 0 ? 0 : (info.rowPitch / RHIUtil::GetFormatSize(imageSlot.info.format)),
            .bufferImageHeight = info.imageExtent.y,
            .imageSubresource = {
                .aspectMask = imageSlot.aspectFlags,
                .mipLevel = info.imageSlice.mipLevel,
                .baseArrayLayer = info.imageSlice.baseArrayLayer,
                .layerCount = info.imageSlice.layerCount,
            },
            .imageOffset = { (int32_t)info.imageOffset.x, (int32_t)info.imageOffset.y, (int32_t)info.imageOffset.z },
            .imageExtent = { info.imageExtent.x, info.imageExtent.y, info.imageExtent.z }
        };
        vkCmdCopyBufferToImage(mCommandBuffer, mDevice->Slot(info.buffer).vkBuffer,
            imageSlot.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanCommandBuffer::CopyImageToBuffer(const CopyImageToBufferInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplImageSlot& imageSlot = mDevice->Slot(info.image);

        const VkBufferImageCopy region = {
            .bufferOffset = info.bufferOffset,
            .bufferRowLength = info.rowPitch / RHIUtil::GetFormatSize(imageSlot.info.format),
            .bufferImageHeight = info.imageExtent.y,
            .imageSubresource = {
                .aspectMask = imageSlot.aspectFlags,
                .mipLevel = info.imageSlice.mipLevel,
                .baseArrayLayer = info.imageSlice.baseArrayLayer,
                .layerCount = info.imageSlice.layerCount,
            },
            .imageOffset = { (int32_t)info.imageOffset.x, (int32_t)info.imageOffset.y, (int32_t)info.imageOffset.z },
            .imageExtent = { info.imageExtent.x, info.imageExtent.y, info.imageExtent.z }
        };
        vkCmdCopyImageToBuffer(mCommandBuffer, imageSlot.vkImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mDevice->Slot(info.buffer).vkBuffer, 1, &region);
    }

    void VulkanCommandBuffer::CopyImageToImage(const CopyImageToImageInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplImageSlot& srcImageSlot = mDevice->Slot(info.srcImage);
        ImplImageSlot& dstImageSlot = mDevice->Slot(info.dstImage);

        const VkImageCopy region = {
            .srcSubresource = {
                .aspectMask = srcImageSlot.aspectFlags,
                .mipLevel = info.srcImageSlice.mipLevel,
                .baseArrayLayer = info.srcImageSlice.baseArrayLayer,
                .layerCount = info.srcImageSlice.layerCount,
            },
            .srcOffset = { info.srcOffset.x, (int32_t)info.srcOffset.y, (int32_t)info.srcOffset.z },
            .dstSubresource = {
                .aspectMask = dstImageSlot.aspectFlags,
                .mipLevel = info.dstImageSlice.mipLevel,
                .baseArrayLayer = info.dstImageSlice.baseArrayLayer,
                .layerCount = info.dstImageSlice.layerCount,
            },
            .dstOffset = { (int32_t)info.dstOffset.x, (int32_t)info.dstOffset.y, (int32_t)info.dstOffset.z },
            .extent = { info.extent.x, info.extent.y, info.extent.z }
        };
        vkCmdCopyImage(mCommandBuffer,
            srcImageSlot.vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImageSlot.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);
    }
    void VulkanCommandBuffer::BlitImageToImage(const BlitImageToImageInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplImageSlot& srcImageSlot = mDevice->Slot(info.srcImage);
        ImplImageSlot& dstImageSlot = mDevice->Slot(info.dstImage);

        const VkImageBlit region{
            .srcSubresource = {
                .aspectMask = srcImageSlot.aspectFlags,
                .mipLevel = info.srcImageSlice.mipLevel,
                .baseArrayLayer = info.srcImageSlice.baseArrayLayer,
                .layerCount = info.srcImageSlice.layerCount,
            },
            .srcOffsets = {
                { info.srcImageRect.x, info.srcImageRect.y, 0 },
                { info.srcImageRect.x + info.srcImageRect.width, info.srcImageRect.y + info.srcImageRect.height, 1 },
            },
            .dstSubresource = {
                .aspectMask = dstImageSlot.aspectFlags,
                .mipLevel = info.dstImageSlice.mipLevel,
                .baseArrayLayer = info.dstImageSlice.baseArrayLayer,
                .layerCount = info.dstImageSlice.layerCount,
            },
            .dstOffsets = {
                { info.dstImageRect.x, info.dstImageRect.y, 0 },
                { info.dstImageRect.x + info.dstImageRect.width, info.dstImageRect.y + info.dstImageRect.height, 1 },
            },
        };

        vkCmdBlitImage(mCommandBuffer, srcImageSlot.vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImageSlot.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, ToVkFilter(info.filter));
    }

    void VulkanCommandBuffer::ClearUnorderedAccessView(const ClearUnorderedAccessViewInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        auto& uav = mDevice->Slot(info.view);

        if (eastl::holds_alternative<BufferResourceInfo>(uav.info)) {
            auto& bufferInfo = uav.descriptor.Get<VkDescriptorBufferInfo>();
            vkCmdFillBuffer(mCommandBuffer, bufferInfo.buffer,
                bufferInfo.offset, bufferInfo.range, info.clearValue.uint32[0]);
        } else if (eastl::holds_alternative<ImageResourceInfo>(uav.info)) {
            auto& imageViewInfo = eastl::get<ImageResourceInfo>(uav.info);
            auto& imageInfo = mDevice->Slot(imageViewInfo.image);
            VkImage image = imageInfo.vkImage;
            VkImageSubresourceRange range = {
                .aspectMask = imageInfo.aspectFlags,
                .baseMipLevel = imageViewInfo.slice.baseMipLevel,
                .levelCount = imageViewInfo.slice.levelCount,
                .baseArrayLayer = imageViewInfo.slice.baseArrayLayer,
                .layerCount = imageViewInfo.slice.layerCount,
            };
            // this is actually a UAV, so it's expected to be in the UAV layout (general in this vulkan RHI)
            if (range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                VkClearColorValue clear;
                memcpy(&clear, &info.clearValue, 16);
                vkCmdClearColorImage(mCommandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &range);
            }
            // else {
            //     VkClearDepthStencilValue clear {
            //         .depth = clear.depth
            //         ;
            //     vkCmdClearDepthStencilImage(mCommandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &clear.depthStencil, 1, &range);
            // }
        } else {
            ASSERT(false, "Bad unordered access view bound!");
        }
    }

    void VulkanCommandBuffer::UpdateBuffer(const UpdateBufferInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();
        auto& bufferSlot = mDevice->Slot(info.buffer);
        vkCmdUpdateBuffer(
            mCommandBuffer,
            bufferSlot.vkBuffer,
            info.region.offset,
            eastl::min(bufferSlot.info.size, info.region.size),
            info.data);
    }


    void VulkanCommandBuffer::BufferBarrier(const BufferMemoryBarrierInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.buffer);

        const usize trueSize = info.region.size == PYRO_MAX_SIZE ? (bufferSlot.info.size - info.region.offset) : info.region.size;
        VkBufferMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = ToVkPipelineStageFlags(info.srcAccess.stages),
            .srcAccessMask = ToVkAccessTypeFlags(info.srcAccess.type),
            .dstStageMask = ToVkPipelineStageFlags(info.dstAccess.stages),
            .dstAccessMask = ToVkAccessTypeFlags(info.dstAccess.type),
            .buffer = bufferSlot.vkBuffer,
            .offset = info.region.offset,
            .size = trueSize,
        };

        mBufferBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::ImageBarrier(const ImageMemoryBarrierInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplImageSlot& imageSlot = mDevice->Slot(info.image);

        VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = ToVkPipelineStageFlags(info.srcAccess.stages),
            .srcAccessMask = ToVkAccessTypeFlags(info.srcAccess.type),
            .dstStageMask = ToVkPipelineStageFlags(info.dstAccess.stages),
            .dstAccessMask = ToVkAccessTypeFlags(info.dstAccess.type),
            .oldLayout = ToVkImageLayout(info.srcLayout),
            .newLayout = ToVkImageLayout(info.dstLayout),
            .image = imageSlot.vkImage,
            .subresourceRange = {
                .aspectMask = imageSlot.aspectFlags,
                .baseMipLevel = info.imageSlice.baseMipLevel,
                .levelCount = info.imageSlice.levelCount,
                .baseArrayLayer = info.imageSlice.baseArrayLayer,
                .layerCount = info.imageSlice.layerCount,
            },
        };

        mImageBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::TransferBufferOwnership(Buffer buffer, ICommandQueue* dstQueue) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplBufferSlot& bufferSlot = mDevice->Slot(buffer);

        VkBufferMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .buffer = bufferSlot.vkBuffer,
        };
        ASSERT(dstQueue != nullptr && mQueue != dstQueue, "Queue ownerships must define BOTH a correct SRC and DST DIFFERENT ICommandQueue's!!");
        barrier.srcQueueFamilyIndex = mQueue->GetQueueFamily();
        barrier.dstQueueFamilyIndex = static_cast<VulkanCommandQueue*>(dstQueue)->GetQueueFamily();

        mBufferBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::TransferImageOwnership(Image image, ICommandQueue* dstQueue) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplImageSlot& imageSlot = mDevice->Slot(image);

        VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .image = imageSlot.vkImage,
            .subresourceRange = {
                .aspectMask = imageSlot.aspectFlags,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };
        ASSERT(dstQueue != nullptr && mQueue != dstQueue, "Queue ownerships must define BOTH a correct SRC and DST DIFFERENT ICommandQueue's!!");
        barrier.srcQueueFamilyIndex = mQueue->GetQueueFamily();
        barrier.dstQueueFamilyIndex = static_cast<VulkanCommandQueue*>(dstQueue)->GetQueueFamily();

        mImageBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::AcquireBufferOwnership(Buffer buffer, ICommandQueue* srcQueue) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplBufferSlot& bufferSlot = mDevice->Slot(buffer);

        VkBufferMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .buffer = bufferSlot.vkBuffer,
        };
        ASSERT(srcQueue != nullptr && mQueue != srcQueue, "Queue ownerships must define BOTH a correct SRC and DST DIFFERENT ICommandQueue's!!");
        barrier.srcQueueFamilyIndex = static_cast<VulkanCommandQueue*>(srcQueue)->GetQueueFamily();
        barrier.dstQueueFamilyIndex = mQueue->GetQueueFamily();

        mBufferBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::AcquireImageOwnership(Image image, ICommandQueue* srcQueue) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        ImplImageSlot& imageSlot = mDevice->Slot(image);

        VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .image = imageSlot.vkImage,
            .subresourceRange = {
                .aspectMask = imageSlot.aspectFlags,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };
        ASSERT(srcQueue != nullptr && mQueue != srcQueue, "Queue ownerships must define BOTH a correct SRC and DST DIFFERENT ICommandQueue's!!");
        barrier.srcQueueFamilyIndex = static_cast<VulkanCommandQueue*>(srcQueue)->GetQueueFamily();
        barrier.dstQueueFamilyIndex = mQueue->GetQueueFamily();

        mImageBarriers.push_back(barrier);
    }

    void VulkanCommandBuffer::DestroyMemoryBlockDeferred(MemoryBlock memory) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(memory),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<MemoryBlock>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyBufferDeferred(Buffer buffer) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(buffer),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<Buffer>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyImageDeferred(Image image) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(image),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<Image>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyShaderResourceDeferred(ShaderResourceId srv) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(srv),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<ShaderResourceId>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyUnorderedAccessDeferred(UnorderedAccessId uav) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(uav),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<UnorderedAccessId>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroySamplerDeferred(SamplerId sampler) {
        mZombieInfo->zombies.push_back({
            .resource = eastl::bit_cast<void*>(sampler),
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = eastl::bit_cast<SamplerId>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyRenderTargetDeferred(RenderTarget renderTarget) {
        mZombieInfo->zombies.push_back({
            .resource = renderTarget,
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = reinterpret_cast<RenderTarget>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyRasterPipelineDeferred(RasterPipeline pipeline) {
        mZombieInfo->zombies.push_back({
            .resource = pipeline,
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = reinterpret_cast<RasterPipeline>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::DestroyComputePipelineDeferred(ComputePipeline pipeline) {
        mZombieInfo->zombies.push_back({
            .resource = pipeline,
            .deleter = [](VulkanDevice* device, void* resource) {
                auto x = reinterpret_cast<ComputePipeline>(resource);
                device->Destroy(x);
            },
        });
    }

    void VulkanCommandBuffer::InvalidateTimestampQuery(const InvalidateTimestampQueryInfo& info) {
        vkCmdResetQueryPool(mCommandBuffer, static_cast<VulkanTimestampQueryPool*>(info.queryPool)->GetVkQueryPool(), info.firstQuery, info.queryCount);
    }

    void VulkanCommandBuffer::WriteTimestamp(const WriteTimestampInfo& info) {
        vkCmdWriteTimestamp2(mCommandBuffer, ToVkPipelineStageFlags(info.stage), static_cast<VulkanTimestampQueryPool*>(info.queryPool)->GetVkQueryPool(), info.queryIndex);
    }

    void VulkanCommandBuffer::BeginLabel(const CommandLabelInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        if (!vkCmdBeginDebugUtilsLabelEXT)
            return;
        // avoid leaking through debug barriers
        FlushBarriers();

        VkDebugUtilsLabelEXT label = {};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = info.name.data();
        label.color[0] = info.labelColor.r;
        label.color[1] = info.labelColor.g;
        label.color[2] = info.labelColor.b;
        label.color[3] = info.labelColor.a;

        vkCmdBeginDebugUtilsLabelEXT(mCommandBuffer, &label);
    }

    void VulkanCommandBuffer::EndLabel() {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        if (!vkCmdEndDebugUtilsLabelEXT)
            return;
        // avoid leaking through debug barriers
        FlushBarriers();
        vkCmdEndDebugUtilsLabelEXT(mCommandBuffer);
    }

    void VulkanCommandBuffer::BeginRenderPass(const RenderPassBeginInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .flags = 0,
            .renderArea = {
                .offset = { info.renderArea.x, info.renderArea.y },
                .extent = { static_cast<u32>(info.renderArea.width), static_cast<u32>(info.renderArea.height) },
            },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<u32>(info.colorAttachments.size()),
        };

        static auto ToRenderingAttachmentInfo = [](const ColorAttachmentInfo& info) {
            VkRenderingAttachmentInfo attachment{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = eastl::bit_cast<VulkanRenderTarget*>(info.target)->GetVkImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .loadOp = ToVkLoadOp(info.loadOp),
                .storeOp = ToVkStoreOp(info.storeOp),
            };
            memcpy(&attachment.clearValue.color, &info.clearValue, 16);
            if (info.resolve.has_value()) {
                attachment.resolveMode = ToVkResolveMode(info.resolve->mode);
                attachment.resolveImageView = eastl::bit_cast<VulkanRenderTarget*>(info.resolve->target)->GetVkImageView();
                attachment.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            }
            return attachment;
        };

        VkRenderingAttachmentInfo colorAttachments[8];
        for (i32 i = 0; i < info.colorAttachments.size(); ++i) {
            colorAttachments[i] = ToRenderingAttachmentInfo(info.colorAttachments[i]);
        }
        if (info.colorAttachments.size() > 0) {
            renderingInfo.pColorAttachments = colorAttachments;
        }
        VkRenderingAttachmentInfo depthAttachment;
        VkRenderingAttachmentInfo stencilAttachment;
        if (info.depthStencilAttachment.has_value()) {
            RenderTargetFlags flags = mDevice->GetRenderTargetInfo(info.depthStencilAttachment->target).flags;
            if (flags & RenderTargetFlagBits::DEPTH_TARGET) {
                depthAttachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = eastl::bit_cast<VulkanRenderTarget*>(info.depthStencilAttachment->target)->GetVkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .loadOp = ToVkLoadOp(info.depthStencilAttachment->depthLoadOp),
                    .storeOp = ToVkStoreOp(info.depthStencilAttachment->depthStoreOp),
                    .clearValue = { .depthStencil = { .depth = info.depthStencilAttachment->clearValue.depth } },
                };
                renderingInfo.pDepthAttachment = &depthAttachment;
            }
            if (flags & RenderTargetFlagBits::STENCIL_TARGET) {
                stencilAttachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = eastl::bit_cast<VulkanRenderTarget*>(info.depthStencilAttachment->target)->GetVkImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .loadOp = ToVkLoadOp(info.depthStencilAttachment->stencilLoadOp),
                    .storeOp = ToVkStoreOp(info.depthStencilAttachment->stencilStoreOp),
                    .clearValue = { .depthStencil = { .stencil = info.depthStencilAttachment->clearValue.stencil } },
                };
                renderingInfo.pStencilAttachment = &stencilAttachment;
            }
        }

        vkCmdBeginRendering(mCommandBuffer, &renderingInfo);

        VkViewport viewport = {
            .x = static_cast<float>(info.renderArea.x),
            .y = static_cast<float>(info.renderArea.y),
            .width = static_cast<float>(info.renderArea.width),
            .height = static_cast<float>(info.renderArea.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        VkRect2D scissor = {
            .offset = { info.renderArea.x, info.renderArea.y },
            .extent = { static_cast<u32>(info.renderArea.width), static_cast<u32>(info.renderArea.height) },
        };
        vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandBuffer::EndRenderPass() {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        // TODO is this relevant for EndRenderPass?
        FlushBarriers();

        vkCmdEndRendering(mCommandBuffer);
    }

    void VulkanCommandBuffer::PushConstantVPtr(const PushConstantInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        ASSERT(info.size < Limits::MAX_PUSH_CONSTANT_SIZE, "Push constant is too big!");
        ASSERT(PYRO_VERIFY_ALIGNMENT(info.size, 4), "Push constants must be DWord aligned!");
        ASSERT(PYRO_VERIFY_ALIGNMENT(info.offset, 4), "Push constants must be DWord aligned!");

        vkCmdPushConstants(mCommandBuffer, mDevice->GetResourceTable().mPipelineLayout,
            VK_SHADER_STAGE_ALL, info.offset, info.size, info.data);
    }

    void VulkanCommandBuffer::SetUnorderedAccessView(const SetUnorderedAccessViewInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ASSERT(info.slot < Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS, "Invalid unordered access view slot!");
        auto& uav = mDevice->Slot(info.view);

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = VK_NULL_HANDLE,
            .dstBinding = info.slot,
            .descriptorCount = 1
        };
        if (eastl::holds_alternative<BufferResourceInfo>(uav.info)) {
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &uav.descriptor.Get<VkDescriptorBufferInfo>();
            write.dstBinding += 8;
        } else if (eastl::holds_alternative<ImageResourceInfo>(uav.info)) {
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write.pImageInfo = &uav.descriptor.Get<VkDescriptorImageInfo>();
            write.dstBinding += 20;
        } else {
            ASSERT(false, "Bad unordered access view bound!");
        }
        vkCmdPushDescriptorSetKHR(mCommandBuffer, ToVkPipelineBindPoint(info.bindPoint),
            mDevice->GetResourceTable().mPipelineLayout, 1, 1, &write);
    }

    void VulkanCommandBuffer::SetUniformBufferView(const SetUniformBufferViewInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ASSERT(info.slot < Limits::MAX_UNIFORM_BUFFER_VIEW_SLOTS, "Invalid uniform buffer slot!");
        ImplBufferSlot& bufferSlot = mDevice->Slot(info.buffer);
        ASSERT(bufferSlot.info.usage & BufferUsageFlagBits::UNIFORM_BUFFER, "Buffer must have been created with uniform buffer capabilities!");

        VkDescriptorBufferInfo bufferInfo{
            .buffer = bufferSlot.vkBuffer,
            .offset = info.region.offset,
            .range = info.region.size
        };
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = VK_NULL_HANDLE,
            .dstBinding = info.slot,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        };
        vkCmdPushDescriptorSetKHR(mCommandBuffer, ToVkPipelineBindPoint(info.bindPoint),
            mDevice->GetResourceTable().mPipelineLayout, 1, 1, &write);
    }

    void VulkanCommandBuffer::SetRasterPipeline(RasterPipeline pipeline) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, eastl::bit_cast<VulkanRasterPipeline*>(pipeline)->GetVkPipeline());
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mDevice->GetResourceTable().mPipelineLayout,
            0, 1, &mDevice->GetResourceTable().mBindlessDescriptorSet, 0, nullptr);
    }

    void VulkanCommandBuffer::SetComputePipeline(ComputePipeline pipeline) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, eastl::bit_cast<VulkanComputePipeline*>(pipeline)->GetVkPipeline());
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mDevice->GetResourceTable().mPipelineLayout,
            0, 1, &mDevice->GetResourceTable().mBindlessDescriptorSet, 0, nullptr);
    }

    void VulkanCommandBuffer::SetViewport(const ViewportInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        VkViewport viewport = {
            .x = info.x,
            .y = info.y,
            .width = info.width,
            .height = info.height,
            .minDepth = info.minDepth,
            .maxDepth = info.maxDepth,
        };
        vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
    }

    void VulkanCommandBuffer::SetScissor(const Rect2D& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");

        VkRect2D scissor = {
            .offset = { info.x, info.y },
            .extent = { static_cast<u32>(info.width), static_cast<u32>(info.height) },
        };
        vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandBuffer::SetIndexBuffer(const SetIndexBufferInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.buffer);

        vkCmdBindIndexBuffer(
            mCommandBuffer,
            bufferSlot.vkBuffer,
            info.offset,
            ToVkIndexType(info.indexType));
    }

    void VulkanCommandBuffer::SetVertexBuffer(const SetVertexBufferInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.buffer);

        VkBuffer buffers[1]{
            bufferSlot.vkBuffer
        };
        VkDeviceSize offsets[1]{
            static_cast<VkDeviceSize>(info.offset)
        };
        vkCmdBindVertexBuffers(mCommandBuffer, info.slot, PYRO_ARRAY_SIZE(buffers), buffers, offsets);
    }

    void VulkanCommandBuffer::Draw(const DrawInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        vkCmdDraw(mCommandBuffer, info.vertexCount, info.instanceCount, info.firstVertex, info.firstInstance);
    }

    void VulkanCommandBuffer::DrawIndexed(const DrawIndexedInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        vkCmdDrawIndexed(mCommandBuffer, info.indexCount, info.instanceCount, info.firstIndex, info.vertexOffset, info.firstInstance);
    }

    void VulkanCommandBuffer::DrawIndirect(const DrawIndirectInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.indirectBuffer);

        vkCmdDrawIndirect(
            mCommandBuffer,
            bufferSlot.vkBuffer,
            info.indirectBufferOffset,
            info.drawCount,
            info.drawCommandStride);
    }

    void VulkanCommandBuffer::DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.indirectBuffer);

        vkCmdDrawIndexedIndirect(
            mCommandBuffer,
            bufferSlot.vkBuffer,
            info.indirectBufferOffset,
            info.drawCount,
            info.drawCommandStride);
    }

    void VulkanCommandBuffer::Dispatch(const DispatchInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        vkCmdDispatch(mCommandBuffer, info.x, info.y, info.z);
    }

    void VulkanCommandBuffer::DispatchIndirect(const DispatchIndirectInfo& info) {
        ASSERT(mCompleted == false, "can not record commands to completed command list");
        FlushBarriers();

        ImplBufferSlot& bufferSlot = mDevice->Slot(info.indirectBuffer);
        vkCmdDispatchIndirect(
            mCommandBuffer,
            bufferSlot.vkBuffer,
            info.indirectBufferOffset);
    }

    void VulkanCommandBuffer::BuildBLAS(const BuildBLASInfo& info) {

    }

    void VulkanCommandBuffer::BuildTLAS(const BuildTLASInfo& info) {

    }

    void VulkanCommandBuffer::Complete() {
        ASSERT(mCompleted == false, "can only complete uncompleted command list");
        FlushBarriers();

        mCompleted = true;

        vkEndCommandBuffer(mCommandBuffer);
    }

    void VulkanCommandBuffer::FlushBarriers() {
        if (!mBufferBarriers.empty() || !mImageBarriers.empty()) {
            VkDependencyInfo dependencies = {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                .memoryBarrierCount = 0,
                .pMemoryBarriers = nullptr,
                .bufferMemoryBarrierCount = static_cast<u32>(mBufferBarriers.size()),
                .pBufferMemoryBarriers = mBufferBarriers.data(),
                .imageMemoryBarrierCount = static_cast<u32>(mImageBarriers.size()),
                .pImageMemoryBarriers = mImageBarriers.data(),
            };
            vkCmdPipelineBarrier2(mCommandBuffer, &dependencies);

            mBufferBarriers.clear();
            mImageBarriers.clear();
        }
    }
} // namespace PyroshockStudios::RHIVulkan