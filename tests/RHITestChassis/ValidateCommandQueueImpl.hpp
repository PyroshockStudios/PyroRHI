#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;


TEST_F(RHI_CONTEXT_FIXTURE_NAME, QueueValid) {
    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());
    ASSERT_GT(queues[0]->GetTimestampTickPeriodNs(), 0ULL);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, EmptyQueueSubmitDoesNotError) {
    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queues[0];
    ASSERT_NE(submitInfo.queue, nullptr);
    ASSERT_GT(queues[0]->GetTimestampTickPeriodNs(), 0ULL);

    mDevice->SubmitQueue(submitInfo);
    mDevice->WaitIdle();
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CopyQueueToGraphicsQueueSync) {
    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());

    ICommandQueue** pGraphicsQueue = eastl::find_if(queues.begin(), queues.end(),
        [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::GRAPHICS; });
    ASSERT_NE(pGraphicsQueue, nullptr);
    ICommandQueue** pCopyQueue = eastl::find_if(queues.begin(), queues.end(),
        [pGraphicsQueue](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER && q != *pGraphicsQueue; });
    if (pCopyQueue == queues.end()) {
        GTEST_LOG_(INFO) << "Device does not support seperate graphics and copy queues, skipping test...";
        return;
    }

    Semaphore waitForCopyQueueSemaphore = mDevice->CreateSemaphore({ .name = "waitForCopyQueueSemaphore" });

    eastl::array signalSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForCopyQueueSemaphore,
            .stage = PipelineStageFlagBits::COPY,
        }
    };

    CommandQueueSubmitInfo copySubmitInfo = {};
    copySubmitInfo.signalSemaphores = signalSemaphores;
    copySubmitInfo.queue = *pCopyQueue;

    eastl::array waitSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForCopyQueueSemaphore,
            .stage = PipelineStageFlagBits::VERTEX_INPUT,
        }
    };

    CommandQueueSubmitInfo graphicsSubmitInfo = {};
    graphicsSubmitInfo.waitSemaphores = waitSemaphores;
    graphicsSubmitInfo.queue = *pGraphicsQueue;

    // Create images!
    ImageInfo imageInfo{};
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.size = { 256, 256, 1 };
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    Image src = mDevice->CreateImage(imageInfo);
    // using a blit for the graphics commands since these don't require us to build any pipelines :D
    imageInfo.usage = ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::BLIT_SRC;
    Image dst = mDevice->CreateImage(imageInfo);
    imageInfo.usage = ImageUsageFlagBits::BLIT_DST;
    Image graphicsDest = mDevice->CreateImage(imageInfo);

    // Record copy commands
    {
        ICommandBuffer* copyCommands = (*pCopyQueue)->GetCommandBuffer({});

        CopyImageToImageInfo copyInfo{};
        copyInfo.srcImage = src;
        copyInfo.dstImage = dst;
        copyInfo.extent = imageInfo.size;

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

        // Acquire queue ownership transfer! Very important!
        EXPECT_NO_THROW(graphicsCommands->AcquireImageOwnership(dst, *pCopyQueue));

        BlitImageToImageInfo blitInfo{};
        blitInfo.srcImage = dst;
        blitInfo.dstImage = graphicsDest;
        blitInfo.srcImageRect = Rect2D::Cut({ imageInfo.size.x, imageInfo.size.y });
        blitInfo.dstImageRect = Rect2D::Cut({ imageInfo.size.x, imageInfo.size.y });

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
    ASSERT_FALSE(queues.empty());

    ICommandQueue** pGraphicsQueue = eastl::find_if(queues.begin(), queues.end(),
        [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::GRAPHICS; });
    ASSERT_NE(pGraphicsQueue, nullptr);
    ICommandQueue** pComputeQueue = eastl::find_if(queues.begin(), queues.end(),
        [pGraphicsQueue](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::COMPUTE && q != *pGraphicsQueue; });
    if (pComputeQueue == queues.end()) {
        GTEST_LOG_(INFO) << "Device does not support seperate graphics and compute queues, skipping test...";
        return;
    }

    Semaphore waitForComputeQueueSemaphore = mDevice->CreateSemaphore({ .name = "waitForComputeQueueSemaphore" });

    eastl::array signalSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForComputeQueueSemaphore,
            .stage = PipelineStageFlagBits::CLEAR,
        }
    };

    CommandQueueSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.signalSemaphores = signalSemaphores;
    computeSubmitInfo.queue = *pComputeQueue;

    eastl::array waitSemaphores = {
        SemaphoreSubmitInfo{
            .semaphore = waitForComputeQueueSemaphore,
            .stage = PipelineStageFlagBits::VERTEX_INPUT,
        }
    };

    CommandQueueSubmitInfo graphicsSubmitInfo = {};
    graphicsSubmitInfo.waitSemaphores = waitSemaphores;
    graphicsSubmitInfo.queue = *pGraphicsQueue;

    // Create images!
    ImageInfo imageInfo{};
    imageInfo.format = Format::RGBA8Unorm;
    imageInfo.usage = ImageUsageFlagBits::UNORDERED_ACCESS;
    Image uavImage = mDevice->CreateImage(imageInfo);
    UnorderedAccessId uav = mDevice->CreateUnorderedAccess(ImageResourceInfo{ .image = uavImage });

    // Record compute commands
    {
        ICommandBuffer* computeCommands = (*pComputeQueue)->GetCommandBuffer({});

        EXPECT_NO_THROW(computeCommands->ImageBarrier({
            .image = uavImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::CLEAR_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::UnorderedAccess,
        }));

        EXPECT_NO_THROW(computeCommands->ClearUnorderedAccessView({ .view = uav, .clearValue = { 0, 0, 0, 0 } }));

        // Transfer queue ownership! Very important!
        EXPECT_NO_THROW(computeCommands->TransferImageOwnership(uavImage, *pGraphicsQueue));

        computeCommands->Complete();
        (*pComputeQueue)->SubmitCommandBuffer(computeCommands);
    }
    // Record graphics commands
    {
        ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});

        // Acquire queue ownership transfer! Very important!
        EXPECT_NO_THROW(graphicsCommands->AcquireImageOwnership(uavImage, *pComputeQueue));

        // No need for an image barrier, since the layout does not change, only a queue ownership ocurred!
        EXPECT_NO_THROW(graphicsCommands->ClearUnorderedAccessView({ .view = uav, .clearValue = { 1, 0, 1, 1 } }));

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
    ASSERT_FALSE(queues.empty());
    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queues[0];

    ICommandBuffer* commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "test1 cmds" });
    submitInfo.queue->SubmitCommandBuffer(commandBuffer);
    mDevice->SubmitQueue(submitInfo);
    submitInfo.queue->WaitIdle();
    commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "test2 cmds" });
    submitInfo.queue->SubmitCommandBuffer(commandBuffer);
    mDevice->SubmitQueue(submitInfo);
    mDevice->WaitIdle();
}