#include "Helpers/ValidationFixture.hpp"
#include <PyroRHI/Api/Util.hpp>
#include <future>
#include <latch>
#include <thread>
using namespace std::chrono_literals;

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferCopyBufferToBufferSucceeds) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 2048;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC | BufferUsageFlagBits::TRANSFER_DST;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer src = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(src);
    Buffer dst = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(dst);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb); // cb is an ICommandBuffer*, which is a handle/pointer

    // Copy a subregion with offsets
    CopyBufferToBufferInfo copyInfo{};
    copyInfo.srcBuffer = src;
    copyInfo.dstBuffer = dst;
    copyInfo.srcOffset = 256; // copy from 256 bytes in
    copyInfo.dstOffset = 512; // copy to 512 bytes in
    copyInfo.size = 512;      // copy 512 bytes � fits easily within 2048
    TRACK_RHI_PARAMETER(copyInfo);


    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = src,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_READ,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferSrc,
    })); // Note: Anonymous structs don't get a macro, but their contents are covered by main structs/handles
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
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] }); // queue is a pointer/handle
    mDevice->WaitIdle();

    mDevice->DestroyBuffer(src);
    mDevice->DestroyBuffer(dst);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferCopyBufferToImageSucceeds) {
    ImageInfo imageInfo{};
    imageInfo.dimensions = ImageDimensions::e3D;
    imageInfo.size = { 16, 16, 4 };
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;
    TRACK_RHI_PARAMETER(imageInfo);

    Image image = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(image);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    CopyBufferToImageInfo info{};
    info.image = image;
    info.imageExtent = { 8, 4, 2 }; // copy smaller region (half width)
    info.imageOffset = { 4, 4, 1 }; // offset to middle of image
    info.rowPitch = 8 * RHIUtil::GetFormatSize(imageInfo.format);
    info.rowPitch = mDevice->ImageSubresourceRowPitch(image, info.rowPitch);

    info.bufferOffset = PYRO_ALIGN(128, mDevice->Properties().bufferImageCopyOffsetAlignment);
    BufferInfo bufferInfo{};
    bufferInfo.size = PYRO_ALIGN(info.rowPitch * info.imageExtent.height * info.imageExtent.depth + info.bufferOffset, 256);
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);
    info.buffer = buffer;
    TRACK_RHI_PARAMETER(info);


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
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferCopyImageToBufferSucceeds) {
    ImageInfo srcImageInfo{};
    srcImageInfo.dimensions = ImageDimensions::e3D;
    srcImageInfo.size = { 16, 16, 4 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(srcImageInfo);

    Image srcImage = mDevice->CreateImage(srcImageInfo);
    TRACK_RHI_HANDLE(srcImage);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    CopyImageToBufferInfo info{};
    info.image = srcImage;
    info.imageExtent = { 8, 4, 2 }; // smaller region
    info.imageOffset = { 2, 3, 1 }; // subregion of the image
    info.rowPitch = 8 * RHIUtil::GetFormatSize(srcImageInfo.format);
    info.rowPitch = mDevice->ImageSubresourceRowPitch(srcImage, info.rowPitch);

    // Calculate safe buffer size = rowPitch * height * depth + offset margin
    BufferInfo bufferInfo{};
    const u64 regionBytes = static_cast<u64>(info.rowPitch) * info.imageExtent.height * info.imageExtent.depth;
    info.bufferOffset = PYRO_ALIGN(128, mDevice->Properties().bufferImageCopyOffsetAlignment);
    bufferInfo.size = PYRO_ALIGN(regionBytes + info.bufferOffset, 256); // <-- ensure valid total size
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(dstBuffer);
    info.buffer = dstBuffer;
    TRACK_RHI_PARAMETER(info);

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

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferCopyImageToImageSucceeds) {
    ImageInfo srcImageInfo{};
    srcImageInfo.size = { 16, 16, 1 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(srcImageInfo);
    Image srcImage = mDevice->CreateImage(srcImageInfo);
    TRACK_RHI_HANDLE(srcImage);

    ImageInfo dstImageInfo{};
    dstImageInfo.size = { 16, 16, 1 };
    dstImageInfo.format = Format::RGBA8Unorm;
    dstImageInfo.usage = ImageUsageFlagBits::TRANSFER_DST;
    TRACK_RHI_PARAMETER(dstImageInfo);
    Image dstImage = mDevice->CreateImage(dstImageInfo);
    TRACK_RHI_HANDLE(dstImage);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    CopyImageToImageInfo info{};
    info.srcImage = srcImage;
    info.dstImage = dstImage;
    info.srcOffset = { 4, 4, 0 }; // copy from center
    info.dstOffset = { 2, 2, 0 }; // copy to different offset
    info.extent = { 8, 8, 1 };    // partial region
    TRACK_RHI_PARAMETER(info);

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


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferClearUAVBufferSucceeds) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 1024;
    bufferInfo.usage = BufferUsageFlagBits::UNORDERED_ACCESS;
    bufferInfo.initialLayout = BufferLayout::UnorderedAccess;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);
    auto uav = mDevice->CreateUnorderedAccess(BufferResourceInfo{ .buffer = buffer });
    TRACK_RHI_HANDLE(uav);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
    TRACK_RHI_PARAMETER(info);


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

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTransferClearUAVImageSucceeds) {
    ImageInfo imageInfo{};
    imageInfo.size = { 256, 256, 1 };
    imageInfo.format = Format::RGBA16Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    imageInfo.name = "";
    TRACK_RHI_PARAMETER(imageInfo);

    Image image = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(image);
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });
    TRACK_RHI_HANDLE(uav);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.1f, 0.2f, 0.3f, 0.4f };
    TRACK_RHI_PARAMETER(info);


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
    TRACK_RHI_PARAMETER(imageInfo);
    Image image = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(image);

    // Prepare the buffer (sized for a small subregion copy like other tests)
    CopyBufferToImageInfo info{};
    info.image = image;
    info.imageExtent = { 16, 16, 1 };
    info.rowPitch = static_cast<u32>(info.imageExtent.width * RHIUtil::GetFormatSize(imageInfo.format));
    info.rowPitch = mDevice->ImageSubresourceRowPitch(image, info.rowPitch);

    BufferInfo bufferInfo{};
    bufferInfo.size = static_cast<u64>(info.rowPitch) * info.imageExtent.height * info.imageExtent.depth;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(bufferInfo);
    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);
    info.buffer = buffer;
    TRACK_RHI_PARAMETER(info);

    // Create a UAV for the image (used in the second command buffer)
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });
    TRACK_RHI_HANDLE(uav);

    // Get the queue & prepare two command buffers
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(queue);
    auto* cbCopy = queue->GetCommandBuffer({ .name = "Copy Commands" });
    TRACK_RHI_HANDLE(cbCopy);
    auto* cbClear = queue->GetCommandBuffer({ .name = "Clear Commands" });
    TRACK_RHI_HANDLE(cbClear);

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
    TRACK_RHI_PARAMETER(clearInfo);


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

TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiThreadedCommandBufferRecordingSubmitOrder) {
    // --- Resource setup ---
    ImageInfo imageInfo{};
    imageInfo.size = { 16, 16, 1 };
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::UNORDERED_ACCESS;
    TRACK_RHI_PARAMETER(imageInfo);
    Image image = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(image);

    CopyBufferToImageInfo copyInfo{};
    copyInfo.image = image;
    copyInfo.imageExtent = { 16, 16, 1 };
    copyInfo.rowPitch = static_cast<u32>(copyInfo.imageExtent.width * RHIUtil::GetFormatSize(imageInfo.format));
    copyInfo.rowPitch = mDevice->ImageSubresourceRowPitch(image, copyInfo.rowPitch);

    BufferInfo bufferInfo{};
    bufferInfo.size = static_cast<u64>(copyInfo.rowPitch) * copyInfo.imageExtent.height * copyInfo.imageExtent.depth;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(bufferInfo);
    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);
    copyInfo.buffer = buffer;
    TRACK_RHI_PARAMETER(copyInfo);

    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });
    TRACK_RHI_HANDLE(uav);

    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(queue);

    // --- Prepare two command buffers ---
    auto* cbCopy = queue->GetCommandBuffer({ .name = "Parallel Copy CB" });
    TRACK_RHI_HANDLE(cbCopy);
    auto* cbClear = queue->GetCommandBuffer({ .name = "Parallel Clear CB" });
    TRACK_RHI_HANDLE(cbClear);

    // --- Multithreaded recording ---
    std::latch recordStart(1);
    std::atomic<bool> copyDone = false;
    std::atomic<bool> clearDone = false;

    std::jthread copyThread([&](std::stop_token) {
        recordStart.wait();

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
        EXPECT_NO_FATAL_FAILURE(cbCopy->CopyBufferToImage(copyInfo));
        cbCopy->Complete();
        copyDone = true;
    });

    std::jthread clearThread([&](std::stop_token) {
        recordStart.wait();

        ClearUnorderedAccessViewInfo clearInfo{};
        clearInfo.view = uav;
        clearInfo.clearValue = { 0.25f, 0.5f, 0.75f, 1.0f };
        TRACK_RHI_PARAMETER(clearInfo);


        EXPECT_NO_FATAL_FAILURE(cbClear->ImageBarrier({
            .image = image,
            .srcAccess = AccessConsts::TRANSFER_WRITE,
            .dstAccess = AccessConsts::CLEAR_WRITE,
            .srcLayout = ImageLayout::TransferDst,
            .dstLayout = ImageLayout::UnorderedAccess,
        }));
        EXPECT_NO_FATAL_FAILURE(cbClear->ClearUnorderedAccessView(clearInfo));
        cbClear->Complete();
        clearDone = true;
    });

    // Start both recording threads simultaneously
    recordStart.count_down();

    // Wait for both threads to complete recording
    using namespace std::chrono_literals;
    const auto start = std::chrono::steady_clock::now();
    while ((!copyDone || !clearDone) && std::chrono::steady_clock::now() - start < 5s)
        std::this_thread::sleep_for(10ms);

    ASSERT_TRUE(copyDone && clearDone) << "Command buffer recording did not complete within timeout.";
    copyThread.request_stop();
    clearThread.request_stop();

    // --- Sequential submission order check ---
    // Submit copy first, clear second
    queue->SubmitCommandBuffer(cbCopy);
    queue->SubmitCommandBuffer(cbClear);

    // Flush queue (submit all)
    mDevice->SubmitQueue({ .queue = queue });

    // Wait for completion
    mDevice->WaitIdle();

    // --- Cleanup ---
    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyBuffer(buffer);
    mDevice->DestroyImage(image);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsUpdateBuffer64kbSucceeds) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 65536;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.initialLayout = BufferLayout::TransferDst;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

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



TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsUpdateBuffer64kbSucceedsWithOffset) {
    BufferInfo bufferInfo{};
    bufferInfo.size = 65536 + 256;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.initialLayout = BufferLayout::TransferDst;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer buffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(buffer);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

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

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsTimestampQueryPoolResetSetSuccess) {
    ImageInfo imageInfo{};
    imageInfo.size = { 256, 256, 1 };
    imageInfo.format = Format::RGBA16Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    imageInfo.name = "uav test";
    TRACK_RHI_PARAMETER(imageInfo);

    Image image = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(image);
    ITimestampQueryPool* qp = mDevice->CreateTimestampQueryPool({ .queryCount = 2, .name = " qp" });
    TRACK_RHI_HANDLE(qp);
    auto uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = image });
    TRACK_RHI_HANDLE(uav);

    auto* cb = mDevice->GetCommandQueues()[0]->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    ClearUnorderedAccessViewInfo info{};
    info.view = uav;
    info.clearValue = { 0.1f, 0.2f, 0.3f, 0.4f };
    TRACK_RHI_PARAMETER(info);


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


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsDestroyDeferredSuccess) {
    static constexpr i32 NUM_DESTROY_DEFERRED_CYCLES = 16;

    ICommandQueue* cq = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(cq);

    eastl::vector<ICommandBuffer*> cbs{};
    for (i32 i = 0; i < NUM_DESTROY_DEFERRED_CYCLES; ++i) {
        ICommandBuffer* cb = cq->GetCommandBuffer({ .name = "destroy deferred commands #" + eastl::to_string(i) });
        TRACK_RHI_HANDLE(cb);
        cbs.push_back(cb);
    }
    for (i32 i = 0; i < NUM_DESTROY_DEFERRED_CYCLES; ++i) {
        ImageInfo srcImageInfo{};
        srcImageInfo.dimensions = ImageDimensions::e3D;
        srcImageInfo.size = { 16, 16, 4 };
        srcImageInfo.format = Format::RGBA8Unorm;
        srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
        srcImageInfo.name = "src image in flight #" + eastl::to_string(i);
        TRACK_RHI_PARAMETER(srcImageInfo);
        Image srcImage = mDevice->CreateImage(srcImageInfo);
        TRACK_RHI_HANDLE(srcImage);
        ASSERT_TRUE(mDevice->IsValid(srcImage));

        MemoryBlockInfo blockInfo = {};
        blockInfo.size = 800000;
        blockInfo.bufferUsage = BufferUsageFlagBits::TRANSFER_DST;
        blockInfo.name = "Test Memory Block";
        TRACK_RHI_PARAMETER(blockInfo);

        MemoryBlock block = mDevice->CreateMemoryBlock(blockInfo);
        TRACK_RHI_HANDLE(block);
        ASSERT_TRUE(mDevice->IsValid(block));

        BufferInfo bufferInfo{};
        bufferInfo.memoryBlock = block;
        bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
        bufferInfo.name = "dst buffer in flight #" + eastl::to_string(i);

        CopyImageToBufferInfo info{};
        info.image = srcImage;
        info.imageExtent = srcImageInfo.size;
        bufferInfo.size = mDevice->ImageSizeRequirements(srcImage);
        info.rowPitch = bufferInfo.size / srcImageInfo.size.height / srcImageInfo.size.depth;
        info.rowPitch = mDevice->ImageSubresourceRowPitch(srcImage, info.rowPitch);

        bufferInfo.size = srcImageInfo.size.height * srcImageInfo.size.depth * info.rowPitch;
        TRACK_RHI_PARAMETER(bufferInfo);

        Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
        TRACK_RHI_HANDLE(dstBuffer);
        ASSERT_TRUE(mDevice->IsValid(dstBuffer));

        auto* cb = cbs[i];

        // It should still be legal to use these resources after destroying! They are still valid in this frame!
        cb->DestroyDeferred(srcImage);
        cb->DestroyDeferred(dstBuffer);
        cb->DestroyDeferred(block);
        eastl::string framename1 = "Record frame #" + eastl::to_string(i);
        eastl::string framename = "Transition Resources " + eastl::to_string(i);
        cb->BeginLabel({ .name = framename1 });
        cb->BeginLabel({ .name = framename });
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
        cb->EndLabel();
        info.buffer = dstBuffer;
        TRACK_RHI_PARAMETER(info);
        EXPECT_NO_FATAL_FAILURE(cb->CopyImageToBuffer(info));
        cb->EndLabel();

        cb->Complete();
        cq->SubmitCommandBuffer(cb);
        mDevice->SubmitQueue({ .queue = cq });
    }
    mDevice->WaitIdle();
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsClearColorPass) {
    ICommandQueue* q = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(q);

    ImageInfo srcImageInfo{};
    srcImageInfo.dimensions = ImageDimensions::e2D;
    srcImageInfo.size = { 16, 16, 1 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::RENDER_TARGET;
    srcImageInfo.name = "color target";
    TRACK_RHI_PARAMETER(srcImageInfo);
    Image srcImage = mDevice->CreateImage(srcImageInfo);
    TRACK_RHI_HANDLE(srcImage);
    ASSERT_TRUE(mDevice->IsValid(srcImage));

    RenderTarget srcTarget = mDevice->CreateRenderTarget({ .image = srcImage, .flags = RenderTargetFlagBits::COLOR_TARGET, .name = "render target" });
    TRACK_RHI_HANDLE(srcTarget);

    ICommandBuffer* cb = q->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = srcImage,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::COLOR_ATTACHMENT_OUTPUT_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::RenderTarget,
    }));

    RenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.colorAttachments = {
        ColorAttachmentInfo{
            .target = srcTarget,
            .loadOp = AttachmentLoadOp::Clear,
            .clearValue = { 1.0f, 1.0f, 1.0f, 1.0f } },
    };
    rpBeginInfo.renderArea = { .x = 0, .y = 0, .width = 16, .height = 16 };
    TRACK_RHI_PARAMETER(rpBeginInfo);

    EXPECT_NO_FATAL_FAILURE(cb->BeginRenderPass(rpBeginInfo));

    EXPECT_NO_FATAL_FAILURE(cb->EndRenderPass());

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->Destroy(srcTarget);
    mDevice->Destroy(srcImage);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsClearColorPassThenDontCareLoad) {
    ICommandQueue* q = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(q);

    ImageInfo srcImageInfo{};
    srcImageInfo.dimensions = ImageDimensions::e2D;
    srcImageInfo.size = { 16, 16, 1 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::RENDER_TARGET;
    srcImageInfo.name = "color target";
    TRACK_RHI_PARAMETER(srcImageInfo);
    Image srcImage = mDevice->CreateImage(srcImageInfo);
    TRACK_RHI_HANDLE(srcImage);
    ASSERT_TRUE(mDevice->IsValid(srcImage));

    RenderTarget srcTarget = mDevice->CreateRenderTarget({ .image = srcImage, .flags = RenderTargetFlagBits::COLOR_TARGET, .name = "render target" });
    TRACK_RHI_HANDLE(srcTarget);

    ICommandBuffer* cb = q->GetCommandBuffer({});
    TRACK_RHI_HANDLE(cb);

    EXPECT_NO_FATAL_FAILURE(cb->ImageBarrier({
        .image = srcImage,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::COLOR_ATTACHMENT_OUTPUT_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::RenderTarget,
    }));

    RenderPassBeginInfo rpBeginInfo1{};
    rpBeginInfo1.colorAttachments = {
        ColorAttachmentInfo{
            .target = srcTarget,
            .loadOp = AttachmentLoadOp::Clear,
            .storeOp = AttachmentStoreOp::Store,
            .clearValue = { 1.0f, 1.0f, 1.0f, 1.0f } },
    };
    rpBeginInfo1.renderArea = { .x = 0, .y = 0, .width = 16, .height = 16 };
    TRACK_RHI_PARAMETER(rpBeginInfo1);

    EXPECT_NO_FATAL_FAILURE(cb->BeginRenderPass(rpBeginInfo1));

    EXPECT_NO_FATAL_FAILURE(cb->EndRenderPass());


    RenderPassBeginInfo rpBeginInfo2{};
    rpBeginInfo2.colorAttachments = {
        ColorAttachmentInfo{
            .target = srcTarget,
            .loadOp = AttachmentLoadOp::DontCare,
        },
    };
    rpBeginInfo2.renderArea = { .x = 0, .y = 0, .width = 16, .height = 16 };
    TRACK_RHI_PARAMETER(rpBeginInfo2);

    EXPECT_NO_FATAL_FAILURE(cb->BeginRenderPass(rpBeginInfo2));

    EXPECT_NO_FATAL_FAILURE(cb->EndRenderPass());

    cb->Complete();
    mDevice->GetCommandQueues()[0]->SubmitCommandBuffer(cb);
    mDevice->SubmitQueue({ .queue = mDevice->GetCommandQueues()[0] });
    mDevice->WaitIdle();

    mDevice->Destroy(srcTarget);
    mDevice->Destroy(srcImage);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CommandsBlitImageArray) {
    auto* q = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(q);

    // Create images!
    ImageInfo imageInfo{};
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.size = { 256, 256, 1 };
    imageInfo.usage = ImageUsageFlagBits::BLIT_SRC;
    imageInfo.arrayLayerCount = 6;
    TRACK_RHI_PARAMETER(imageInfo);
    Image src = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(src);

    imageInfo.arrayLayerCount = 8;
    imageInfo.usage = ImageUsageFlagBits::BLIT_DST;
    TRACK_RHI_PARAMETER(imageInfo); // Update tracker for modified imageInfo
    Image graphicsDest = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(graphicsDest);


    ICommandBuffer* graphicsCommands = q->GetCommandBuffer({});
    TRACK_RHI_HANDLE(graphicsCommands);

    // we are going to blit from [2, 6) to [4, 8)
    BlitImageToImageInfo blitInfo{};
    blitInfo.srcImage = src;
    blitInfo.dstImage = graphicsDest;
    blitInfo.srcImageBox = Box3D::Cut({ imageInfo.size.width, imageInfo.size.height, 1 });
    blitInfo.dstImageBox = Box3D::Cut({ imageInfo.size.width, imageInfo.size.height, 1 });
    blitInfo.srcImageSlice.baseArrayLayer = 2;
    blitInfo.srcImageSlice.layerCount = 4;
    blitInfo.dstImageSlice.baseArrayLayer = 4;
    blitInfo.dstImageSlice.layerCount = 4;
    TRACK_RHI_PARAMETER(blitInfo);

    EXPECT_NO_THROW(graphicsCommands->ImageBarrier({
        .image = src,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::BLIT_READ,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::BlitSrc,
    }));

    EXPECT_NO_THROW(graphicsCommands->ImageBarrier({
        .image = graphicsDest,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::BLIT_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::BlitDst,
    }));
    EXPECT_NO_THROW(graphicsCommands->BlitImageToImage(blitInfo));

    graphicsCommands->Complete();
    q->SubmitCommandBuffer(graphicsCommands);


    mDevice->SubmitQueue({ .queue = q });

    mDevice->WaitIdle();
    mDevice->DestroyImage(src);
    mDevice->DestroyImage(graphicsDest);
}