#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;


TEST_F(RHI_CONTEXT_FIXTURE_NAME, QueueValid) {
    auto queues = mDevice->GetCommandQueues();
    // Assuming GetCommandQueues() returns a vector of ICommandQueue*
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());
    ASSERT_GT(queues[0]->GetTimestampTickPeriodNs(), 0ULL);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, EmptyQueueSubmitDoesNotError) {
    auto queues = mDevice->GetCommandQueues();
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queues[0];
    TRACK_RHI_PARAMETER(submitInfo);
    TRACK_RHI_HANDLE(submitInfo.queue); // Explicitly track the queue in submitInfo if desired

    ASSERT_NE(submitInfo.queue, nullptr);
    ASSERT_GT(queues[0]->GetTimestampTickPeriodNs(), 0.0);

    mDevice->SubmitQueue(submitInfo);
    mDevice->WaitIdle();
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CopyQueueToGraphicsQueueSync) {
    auto queues = mDevice->GetCommandQueues();
    // Assuming GetCommandQueues() returns a vector of ICommandQueue*
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());

    ICommandQueue** pGraphicsQueue = eastl::find_if(queues.begin(), queues.end(),
        [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::GRAPHICS; });
    ASSERT_NE(pGraphicsQueue, nullptr);
    TRACK_RHI_HANDLE(*pGraphicsQueue); // Track the found graphics queue

    ICommandQueue** pCopyQueue = eastl::find_if(queues.begin(), queues.end(),
        [pGraphicsQueue](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER && q != *pGraphicsQueue; });
    if (pCopyQueue == queues.end()) {
        GTEST_LOG_(INFO) << "Device does not support seperate graphics and copy queues, skipping test...";
        return;
    }
    TRACK_RHI_HANDLE(*pCopyQueue); // Track the found copy queue

    Semaphore waitForCopyQueueSemaphore = mDevice->CreateSemaphore({ .name = "waitForCopyQueueSemaphore" });
    TRACK_RHI_HANDLE(waitForCopyQueueSemaphore);

    eastl::array signalSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForCopyQueueSemaphore,
            .stage = PipelineStageFlagBits::COPY,
        }
    };
    // No direct macro for eastl::array initializer list elements,
    // but the containing submitInfo will be tracked.

    CommandQueueSubmitInfo copySubmitInfo = {};
    copySubmitInfo.signalSemaphores = signalSemaphores;
    copySubmitInfo.queue = *pCopyQueue;
    TRACK_RHI_PARAMETER(copySubmitInfo);
    TRACK_RHI_HANDLE(copySubmitInfo.queue);

    eastl::array waitSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForCopyQueueSemaphore,
            .stage = PipelineStageFlagBits::VERTEX_INPUT,
        }
    };

    CommandQueueSubmitInfo graphicsSubmitInfo = {};
    graphicsSubmitInfo.waitSemaphores = waitSemaphores;
    graphicsSubmitInfo.queue = *pGraphicsQueue;
    TRACK_RHI_PARAMETER(graphicsSubmitInfo);
    TRACK_RHI_HANDLE(graphicsSubmitInfo.queue);

    // Create images!
    ImageInfo imageInfo{};
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.size = { 256, 256, 1 };
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    TRACK_RHI_PARAMETER(imageInfo);
    Image src = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(src);

    // using a blit for the graphics commands since these don't require us to build any pipelines :D
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::BLIT_SRC;
    TRACK_RHI_PARAMETER(imageInfo); // Update tracker for modified imageInfo
    Image dst = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(dst);

    imageInfo.usage = ImageUsageFlagBits::BLIT_DST;
    TRACK_RHI_PARAMETER(imageInfo); // Update tracker for modified imageInfo
    Image graphicsDest = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(graphicsDest);

    // Record copy commands
    {
        ICommandBuffer* copyCommands = (*pCopyQueue)->GetCommandBuffer({});
        TRACK_RHI_HANDLE(copyCommands);

        CopyImageToImageInfo copyInfo{};
        copyInfo.srcImage = src;
        copyInfo.dstImage = dst;
        copyInfo.extent = imageInfo.size;
        TRACK_RHI_PARAMETER(copyInfo);

        EXPECT_NO_THROW(copyCommands->ImageBarrier({
            .image = src,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferSrc,
        }));
        EXPECT_NO_THROW(copyCommands->ImageBarrier({
            .image = dst,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferDst,
        }));

        EXPECT_NO_THROW(copyCommands->CopyImageToImage(copyInfo));

        // Transfer queue ownership! Very important!
        EXPECT_NO_THROW(copyCommands->TransferImageOwnership(dst, *pGraphicsQueue));

        copyCommands->Complete();
        (*pCopyQueue)->SubmitCommandBuffer(copyCommands);
    }
    // Record graphics commands
    {
        ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});
        TRACK_RHI_HANDLE(graphicsCommands);

        // Acquire queue ownership transfer! Very important!
        EXPECT_NO_THROW(graphicsCommands->AcquireImageOwnership(dst, *pCopyQueue));

        BlitImageToImageInfo blitInfo{};
        blitInfo.srcImage = dst;
        blitInfo.dstImage = graphicsDest;
        blitInfo.srcImageBox = Box3D::Cut({ imageInfo.size.width, imageInfo.size.height, 1 }); // Box3D::Cut is a static function or macro, so params don't get a macro
        blitInfo.dstImageBox = Box3D::Cut({ imageInfo.size.width, imageInfo.size.height, 1 });
        TRACK_RHI_PARAMETER(blitInfo);

        EXPECT_NO_THROW(graphicsCommands->ImageBarrier({
            .image = dst,
            .srcAccess = AccessConsts::TRANSFER_WRITE,
            .dstAccess = AccessConsts::BLIT_READ,
            .srcLayout = ImageLayout::TransferDst,
            // The ownership must be done in the graphics queue, since Blit[...] is a graphics queue layout.
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
        (*pGraphicsQueue)->SubmitCommandBuffer(graphicsCommands);
    }

    // While execution is concurrent, submission must be in order
    mDevice->SubmitQueue(copySubmitInfo);
    mDevice->SubmitQueue(graphicsSubmitInfo);

    (*pGraphicsQueue)->WaitIdle();
    // waiting for the graphics queue alone should be fine for destruction
    mDevice->DestroySemaphore(waitForCopyQueueSemaphore);

    mDevice->DestroyImage(src);
    mDevice->DestroyImage(dst);
    mDevice->DestroyImage(graphicsDest);
}



TEST_F(RHI_CONTEXT_FIXTURE_NAME, ComputeQueueToGraphicsQueueSyncUAV) {
    auto queues = mDevice->GetCommandQueues();
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());

    ICommandQueue** pGraphicsQueue = eastl::find_if(queues.begin(), queues.end(),
        [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::GRAPHICS; });
    ASSERT_NE(pGraphicsQueue, nullptr);
    TRACK_RHI_HANDLE(*pGraphicsQueue);
    ICommandQueue** pComputeQueue = eastl::find_if(queues.begin(), queues.end(),
        [pGraphicsQueue](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::COMPUTE && q != *pGraphicsQueue; });
    if (pComputeQueue == queues.end()) {
        GTEST_LOG_(INFO) << "Device does not support seperate graphics and compute queues, skipping test...";
        return;
    }
    TRACK_RHI_HANDLE(*pComputeQueue);

    Semaphore waitForComputeQueueSemaphore = mDevice->CreateSemaphore({ .name = "waitForComputeQueueSemaphore" });
    TRACK_RHI_HANDLE(waitForComputeQueueSemaphore);

    eastl::array signalSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForComputeQueueSemaphore,
            .stage = PipelineStageFlagBits::CLEAR,
        }
    };

    CommandQueueSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.signalSemaphores = signalSemaphores;
    computeSubmitInfo.queue = *pComputeQueue;
    TRACK_RHI_PARAMETER(computeSubmitInfo);
    TRACK_RHI_HANDLE(computeSubmitInfo.queue);

    eastl::array waitSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForComputeQueueSemaphore,
            .stage = PipelineStageFlagBits::VERTEX_INPUT,
        }
    };

    CommandQueueSubmitInfo graphicsSubmitInfo = {};
    graphicsSubmitInfo.waitSemaphores = waitSemaphores;
    graphicsSubmitInfo.queue = *pGraphicsQueue;
    TRACK_RHI_PARAMETER(graphicsSubmitInfo);
    TRACK_RHI_HANDLE(graphicsSubmitInfo.queue);

    // Create images!
    ImageInfo imageInfo{};
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    TRACK_RHI_PARAMETER(imageInfo);
    Image uavImage = mDevice->CreateImage(imageInfo);
    TRACK_RHI_HANDLE(uavImage);
    UnorderedAccessId uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = uavImage });
    TRACK_RHI_HANDLE(uav);

    // Record compute commands
    {
        ICommandBuffer* computeCommands = (*pComputeQueue)->GetCommandBuffer({});
        TRACK_RHI_HANDLE(computeCommands);

        EXPECT_NO_THROW(computeCommands->ImageBarrier({
            .image = uavImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::CLEAR_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::UnorderedAccess,
        }));

        ClearUnorderedAccessViewInfo clearUAVInfo = { .view = uav, .clearValue = { 0, 0, 0, 0 } };
        TRACK_RHI_PARAMETER(clearUAVInfo);
        EXPECT_NO_THROW(computeCommands->ClearUnorderedAccessView(clearUAVInfo));

        // Transfer queue ownership! Very important!
        EXPECT_NO_THROW(computeCommands->TransferImageOwnership(uavImage, *pGraphicsQueue));

        computeCommands->Complete();
        (*pComputeQueue)->SubmitCommandBuffer(computeCommands);
    }
    // Record graphics commands
    {
        ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});
        TRACK_RHI_HANDLE(graphicsCommands);

        // Acquire queue ownership transfer! Very important!
        EXPECT_NO_THROW(graphicsCommands->AcquireImageOwnership(uavImage, *pComputeQueue));

        // No need for an image barrier, since the layout does not change, only a queue ownership ocurred!
        ClearUnorderedAccessViewInfo clearUAVInfo = { .view = uav, .clearValue = { 1, 0, 1, 1 } };
        TRACK_RHI_PARAMETER(clearUAVInfo);
        EXPECT_NO_THROW(graphicsCommands->ClearUnorderedAccessView(clearUAVInfo));

        graphicsCommands->Complete();
        (*pGraphicsQueue)->SubmitCommandBuffer(graphicsCommands);
    }

    // While execution is concurrent, submission must be in order
    mDevice->SubmitQueue(computeSubmitInfo);
    mDevice->SubmitQueue(graphicsSubmitInfo);

    (*pGraphicsQueue)->WaitIdle();
    // waiting for the graphics queue alone should be fine for destruction
    mDevice->DestroySemaphore(waitForComputeQueueSemaphore);

    mDevice->DestroyUnorderedAccess(uav);
    mDevice->DestroyImage(uavImage);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, QueueSubmitAgainIsOkay) {
    auto queues = mDevice->GetCommandQueues();
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());
    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queues[0];
    TRACK_RHI_PARAMETER(submitInfo);
    TRACK_RHI_HANDLE(submitInfo.queue);

    ICommandBuffer* commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "test1 cmds" });
    TRACK_RHI_HANDLE(commandBuffer);
    commandBuffer->Complete();
    submitInfo.queue->SubmitCommandBuffer(commandBuffer);
    // Submit 1
    mDevice->SubmitQueue(submitInfo);
    // Wait
    submitInfo.queue->WaitIdle();
    // Get new commands
    commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "test2 cmds" });
    TRACK_RHI_HANDLE(commandBuffer);
    commandBuffer->Complete();
    submitInfo.queue->SubmitCommandBuffer(commandBuffer);
    // Submit 2
    mDevice->SubmitQueue(submitInfo);
    // Finally wait for queue to finish.
    submitInfo.queue->WaitIdle();
}