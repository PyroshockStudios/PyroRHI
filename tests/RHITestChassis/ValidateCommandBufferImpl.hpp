#include "Helpers/ValidationFixture.hpp"
#include <PyroRHI/Api/Util.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;
TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyBufferToBufferSucceeds) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 2048;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC | BufferUsageFlagBits::TRANSFER_DST;

    Buffer src = mDevice->CreateBuffer(bufferInfo);
    Buffer dst = mDevice->CreateBuffer(bufferInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    // Copy a subregion with offsets
    CopyBufferToBufferInfo copyInfo{};
    copyInfo.srcBuffer = src;
    copyInfo.dstBuffer = dst;
    copyInfo.srcOffset = 256; // copy from 256 bytes in
    copyInfo.dstOffset = 512; // copy to 512 bytes in
    copyInfo.size = 512;      // copy 512 bytes — fits easily within 2048

    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = src,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferSrc,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = dst,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferDst,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->CopyBufferToBuffer(copyInfo));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyBuffer(src);
    mDevice->DestroyBuffer(dst);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyBufferToImageSucceeds) {
    ImageInfo imageInfo{};
    imageInfo.dimensions = ImageDimensions::e3D;
    imageInfo.size = { 16, 16, 4 };
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;
    Image image = mDevice->CreateImage(imageInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    CopyBufferToImageInfo info{};
    info.image = image;
    info.imageExtent = { 8, 4, 2 }; // copy smaller region (half width)
    info.imageOffset = { 4, 4, 1 }; // offset to middle of image
    info.rowPitch = 8 * RHIUtil::GetFormatSize(imageInfo.format);
    info.rowPitch = mDevice->ImageSubresourceRowPitch(image, {}, info.rowPitch);

    info.bufferOffset = PYRO_ALIGN(128, mDevice->GetProperties().bufferImageCopyOffsetAlignment);
    BufferInfo bufferInfo{};
    bufferInfo.size = PYRO_ALIGN(info.rowPitch * info.imageExtent.y * info.imageExtent.z + info.bufferOffset, 256);
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    info.buffer = buffer;

    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = buffer,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferSrc,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::TransferDst,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->CopyBufferToImage(info));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyBuffer(buffer);
    mDevice->DestroyImage(image);
}
TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyImageToBufferSucceeds) {
    ImageInfo srcImageInfo{};
    srcImageInfo.dimensions = ImageDimensions::e3D;
    srcImageInfo.size = { 16, 16, 4 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    Image srcImage = mDevice->CreateImage(srcImageInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    CopyImageToBufferInfo info{};
    info.image = srcImage;
    info.imageExtent = { 8, 4, 2 }; // smaller region
    info.imageOffset = { 2, 3, 1 }; // subregion of the image
    info.rowPitch = 8 * RHIUtil::GetFormatSize(srcImageInfo.format);
    info.rowPitch = mDevice->ImageSubresourceRowPitch(srcImage, {}, info.rowPitch);

    // Calculate safe buffer size = rowPitch * height * depth + offset margin
    BufferInfo bufferInfo{};
    const u64 regionBytes = static_cast<u64>(info.rowPitch) * info.imageExtent.y * info.imageExtent.z;
    info.bufferOffset = PYRO_ALIGN(128, mDevice->GetProperties().bufferImageCopyOffsetAlignment);
    bufferInfo.size = PYRO_ALIGN(regionBytes + info.bufferOffset, 256); // <-- ensure valid total size
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;

    Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
    info.buffer = dstBuffer;

    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = srcImage,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::TransferSrc,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = dstBuffer,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferDst,
    }));

    EXPECT_NO_FATAL_FAILURE(cb->CopyImageToBuffer(info));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyImage(srcImage);
    mDevice->DestroyBuffer(dstBuffer);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferCopyImageToImageSucceeds) {
    ImageInfo srcImageInfo{};
    srcImageInfo.size = { 16, 16, 1 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    Image srcImage = mDevice->CreateImage(srcImageInfo);

    ImageInfo dstImageInfo{};
    dstImageInfo.size = { 16, 16, 1 };
    dstImageInfo.format = Format::RGBA8Unorm;
    dstImageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;
    Image dstImage = mDevice->CreateImage(dstImageInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    CopyImageToImageInfo info{};
    info.srcImage = srcImage;
    info.dstImage = dstImage;
    info.srcOffset = { 4, 4, 0 }; // copy from center
    info.dstOffset = { 2, 2, 0 }; // copy to different offset
    info.extent = { 8, 8, 1 };    // partial region

    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = srcImage,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::TransferSrc,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = dstImage,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::TransferDst,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->CopyImageToImage(info));

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
    bufferInfo.usage = BufferUsageFlagBits::UNORDERED_ACCESS;
    bufferInfo.initialLayout = BufferLayout::UnorderedAccess;

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    auto uav = mDevice->CreateUnorderedAccess(BufferResourceInfo{ .buffer = buffer });

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = buffer,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::CLEAR_WRITE,
        .srcLayout = BufferLayout::UnorderedAccess,
        .dstLayout = BufferLayout::UnorderedAccess,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ClearUnorderedAccessView(info));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyBuffer(buffer);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, TransferClearUAVImageSucceeds) {
    ImageInfo imageInfo{};
    imageInfo.size = { 256, 256, 1 };
    imageInfo.format = Format::RGBA16Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    imageInfo.name = "";

    Image image = mDevice->CreateImage(imageInfo);
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.1f, 0.2f, 0.3f, 0.4f };

    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::CLEAR_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::UnorderedAccess,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ClearUnorderedAccessView(info));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyImage(image);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiCommandBufferSyncSubmit) {
    // Create a small 2D image and a buffer we can copy from
    ImageInfo imageInfo{};
    imageInfo.size = { 16, 16, 1 };
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::UNORDERED_ACCESS;
    Image image = mDevice->CreateImage(imageInfo);

    // Prepare the buffer (sized for a small subregion copy like other tests)
    CopyBufferToImageInfo info{};
    info.image = image;
    info.imageExtent = { 16, 16, 1 };
    info.rowPitch = static_cast<u32>(info.imageExtent.x * RHIUtil::GetFormatSize(imageInfo.format));
    info.rowPitch = mDevice->ImageSubresourceRowPitch(image, {}, info.rowPitch);

    BufferInfo bufferInfo{};
    bufferInfo.size = static_cast<u64>(info.rowPitch) * info.imageExtent.y * info.imageExtent.z;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    info.buffer = buffer;

    // Create a UAV for the image (used in the second command buffer)
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });

    // Get the queue & prepare two command buffers
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    auto* cbCopy = queue->GetCommandBuffer({ .name = "Copy Commands" });
    auto* cbClear = queue->GetCommandBuffer({ .name = "Clear Commands" });

    // --- Command buffer A: copy buffer -> image ---
    EXPECT_NO_FATAL_FAILURE(cbCopy->BufferBarrier({
        .buffer = buffer,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferSrc,
    }));
    EXPECT_NO_FATAL_FAILURE(cbCopy->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::TransferDst,
    }));
    EXPECT_NO_FATAL_FAILURE(cbCopy->CopyBufferToImage(info));
    cbCopy->Complete();

    // Submit the copy command buffer to the queue (but do not SubmitQueue yet)
    queue->SubmitCommandBuffer(cbCopy);

    // --- Command buffer B: transition to UAV and clear ---
    // The image is expected to be in TransferDst after cbCopy. We now transition it
    // to UnorderedAccess for the clear. Follow the same style as your other tests.
    ClearUnorderedAccessViewInfo clearInfo{};
    clearInfo.view = uav;
    clearInfo.clearValue = { 0.25f, 0.5f, 0.75f, 1.0f };

    EXPECT_NO_FATAL_FAILURE(cbClear->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::TRANSFER_WRITE,
        .dstAccess = AccessConsts::CLEAR_WRITE,
        .srcLayout = ImageLayout::TransferDst,
        .dstLayout = ImageLayout::UnorderedAccess,
    }));
    EXPECT_NO_FATAL_FAILURE(cbClear->ClearUnorderedAccessView(clearInfo));
    cbClear->Complete();

    // Submit the clear command buffer to the queue
    queue->SubmitCommandBuffer(cbClear);

    // Now flush the queue (this should submit both cmd buffers in the order they were queued)
    mDevice->SubmitQueue({ .queue = queue });

    // Wait for work to finish and cleanup
    mDevice->WaitIdle();

    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyBuffer(buffer);
    mDevice->DestroyImage(image);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, UpdateBuffer64kbSucceeds) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 65536;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.initialLayout = BufferLayout::TransferDst;

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    eastl::vector<u8> data(65536, static_cast<u8>(76));

    EXPECT_NO_FATAL_FAILURE(cb->UpdateBuffer({
        .buffer = buffer,
        .region = { .size = data.size() },
        .data = data.data(),
    }));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyBuffer(buffer);
}



TEST_F(RHI_CONTEXT_FIXTURE_NAME, UpdateBuffer64kbSucceedsWithOffset) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 65536 + 256;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.initialLayout = BufferLayout::TransferDst;

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});

    eastl::vector<u8> data(65536, static_cast<u8>(76));

    EXPECT_NO_FATAL_FAILURE(cb->UpdateBuffer({
        .buffer = buffer,
        .region = { .offset = 256, .size = data.size() },
        .data = data.data(),
    }));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->DestroyBuffer(buffer);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, TimestampQueryPoolResetSetSuccess) {
    ImageInfo imageInfo{};
    imageInfo.size = { 256, 256, 1 };
    imageInfo.format = Format::RGBA16Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    imageInfo.name = "uav test";

    Image image = mDevice->CreateImage(imageInfo);
    ITimestampQueryPool* qp = mDevice->CreateTimestampQueryPool({ .queryCount = 2, .name = " qp" });
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.1f, 0.2f, 0.3f, 0.4f };

    EXPECT_NO_FATAL_FAILURE(cb->InvalidateTimestampQuery({ .queryPool = qp, .queryCount = 2 }));

    EXPECT_NO_FATAL_FAILURE(cb->WriteTimestamp({
        .queryPool = qp,
        .stage = PipelineStageFlagBits::CLEAR,
        .queryIndex = 0,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::CLEAR_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::UnorderedAccess,
    }));
    EXPECT_NO_FATAL_FAILURE(cb->ClearUnorderedAccessView(info));

    EXPECT_NO_FATAL_FAILURE(cb->WriteTimestamp({
        .queryPool = qp,
        .stage = PipelineStageFlagBits::CLEAR,
        .queryIndex = 1,
    }));

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    eastl::span<const u64> stamps;
    EXPECT_NO_FATAL_FAILURE(stamps = qp->GetTimestamps(0, 2));
    EXPECT_NE(stamps[0], ~(0ULL));
    EXPECT_NE(stamps[1], ~(0ULL));
    ASSERT_GE(stamps[1], stamps[0]);

    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyImage(image);
    mDevice->DestroyTimestampQueryPool(qp);
}