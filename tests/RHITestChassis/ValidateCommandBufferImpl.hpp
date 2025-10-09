#include "Helpers/ValidationFixture.hpp"

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

    TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyBufferToBufferSucceeds) {
        BufferInfo bufferInfo{};
        bufferInfo.size = 1024;
        bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC | BufferUsageFlagBits::TRANSFER_DST;

        Buffer src = mDevice->CreateBuffer(bufferInfo);
        Buffer dst = mDevice->CreateBuffer(bufferInfo);

        auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
        CopyBufferToBufferInfo copyInfo{};
        copyInfo.srcBuffer = src;
        copyInfo.dstBuffer = dst;
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;
        copyInfo.size = bufferInfo.size;

        EXPECT_NO_THROW(cb->BufferBarrier({
            .buffer = src,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ,
            .srcLayout = BufferLayout::Undefined,
            .dstLayout = BufferLayout::TransferSrc,
        }));

        EXPECT_NO_THROW(cb->BufferBarrier({
            .buffer = src,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = BufferLayout::Undefined,
            .dstLayout = BufferLayout::TransferDst,
        }));

        EXPECT_NO_THROW(cb->CopyBufferToBuffer(copyInfo));

        cb->Complete();
        mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
        mDevice->WaitIdle();

        mDevice->DestroyBuffer(src);
        mDevice->DestroyBuffer(dst);
    }

    TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyBufferToImageSucceeds) {

        BufferInfo bufferInfo{};
        bufferInfo.size = 1024;
        bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;

        Buffer buffer = mDevice->CreateBuffer(bufferInfo);

        ImageInfo imageInfo{};
        imageInfo.size = { 16, 16, 1 };
        imageInfo.format = Format::RGBA8Unorm;
        imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;

        Image image = mDevice->CreateImage(imageInfo);

        auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
        CopyBufferToImageInfo info{};
        info.buffer = buffer;
        info.image = image;
        info.imageExtent = imageInfo.size;

        
        EXPECT_NO_THROW(cb->BufferBarrier({
            .buffer = buffer,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ,
            .srcLayout = BufferLayout::Undefined,
            .dstLayout = BufferLayout::TransferSrc,
        }));
        EXPECT_NO_THROW(cb->ImageBarrier({
            .image = image,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferDst,
        }));

        EXPECT_NO_THROW(cb->CopyBufferToImage(info));

        cb->Complete();
        mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
        mDevice->WaitIdle();

        mDevice->DestroyBuffer(buffer);
        mDevice->DestroyImage(image);
    }
    TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyImageToBufferSucceeds) {
        // Create a memory block
        // Create source image
        ImageInfo srcImageInfo{};
        srcImageInfo.size = { 16, 16, 1 };
        srcImageInfo.format = Format::RGBA8Unorm;
        srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
        Image srcImage = mDevice->CreateImage(srcImageInfo);

        // Create destination buffer
        BufferInfo dstBufferInfo{};
        dstBufferInfo.size = 1024;
        dstBufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
        Buffer dstBuffer = mDevice->CreateBuffer(dstBufferInfo);

        auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
        CopyImageToBufferInfo info{};
        info.image = srcImage;
        info.buffer = dstBuffer;
        info.imageExtent = srcImageInfo.size;

        EXPECT_NO_THROW(cb->ImageBarrier({
            .image = srcImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ ,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferSrc,
        }));
        EXPECT_NO_THROW(cb->BufferBarrier({
            .buffer = dstBuffer,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = BufferLayout::Undefined,
            .dstLayout = BufferLayout::TransferDst,
        }));

        EXPECT_NO_THROW(cb->CopyImageToBuffer(info));

        cb->Complete();
        mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
        mDevice->WaitIdle();

        mDevice->DestroyImage(srcImage);
        mDevice->DestroyBuffer(dstBuffer);
    }

    TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyImageToImageSucceeds) {
        // Create source image
        ImageInfo srcImageInfo{};
        srcImageInfo.size = { 16, 16, 1 };
        srcImageInfo.format = Format::RGBA8Unorm;
        srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
        Image srcImage = mDevice->CreateImage(srcImageInfo);

        // Create destination image
        ImageInfo dstImageInfo{};
        dstImageInfo.size = { 16, 16, 1 };
        dstImageInfo.format = Format::RGBA8Unorm;
        dstImageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;
        Image dstImage = mDevice->CreateImage(dstImageInfo);

        auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
        CopyImageToImageInfo info{};
        info.srcImage = srcImage;
        info.dstImage = dstImage;
        info.extent = srcImageInfo.size;

        EXPECT_NO_THROW(cb->ImageBarrier({
            .image = srcImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferSrc,
        }));
        EXPECT_NO_THROW(cb->ImageBarrier({
            .image = dstImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferDst,
        }));
        EXPECT_NO_THROW(cb->CopyImageToImage(info));

        cb->Complete();
        mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
        mDevice->WaitIdle();

        mDevice->DestroyImage(srcImage);
        mDevice->DestroyImage(dstImage);
    }

    TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferClearUAVBufferSucceeds) {
        BufferInfo bufferInfo{};
        bufferInfo.size = 1024;
        bufferInfo.usage = BufferUsageFlagBits::UNORDERED_ACCESS | BufferUsageFlagBits::TRANSFER_DST;

        Buffer buffer = mDevice->CreateBuffer(bufferInfo);
        auto uav = mDevice->CreateUnorderedAccess(BufferResourceInfo{ .buffer = buffer });

        auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
        ClearUnorderedAccessViewInfo info{};
        info.view = uav;
        info.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

        EXPECT_NO_THROW(cb->BufferBarrier({
            .buffer = buffer,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = BufferLayout::Undefined,
            .dstLayout = BufferLayout::TransferDst,
        }));
        EXPECT_NO_THROW(cb->ClearUnorderedAccessView(info));

        cb->Complete();
        mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
        mDevice->WaitIdle();

        mDevice->DestroyUnorderedAccess(uav);
        mDevice->DestroyBuffer(buffer);
    }
