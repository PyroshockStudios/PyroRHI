#include "Helpers/ValidationFixture.hpp"

#include <shared_mutex>
#include <thread> // for concurrent queue submits
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
        GTEST_SKIP() << "Device does not support seperate graphics and copy queues, skipping test...";
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
    ICommandBuffer* copyCommands = (*pCopyQueue)->GetCommandBuffer({});
    {
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
        copySubmitInfo.commands = { &copyCommands, 1 };
    }
    // Record graphics commands
    ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});
    {
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
        graphicsSubmitInfo.commands = { &graphicsCommands, 1 };
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
        GTEST_SKIP() << "Device does not support seperate graphics and compute queues, skipping test...";
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
            .stage = PipelineStageFlagBits::CLEAR,
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
    ICommandBuffer* computeCommands = (*pComputeQueue)->GetCommandBuffer({});
    {
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
        computeSubmitInfo.commands = { &computeCommands, 1 };
    }
    // Record graphics commands
    ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});
    {
        TRACK_RHI_HANDLE(graphicsCommands);

        // Acquire queue ownership transfer! Very important!
        EXPECT_NO_THROW(graphicsCommands->AcquireImageOwnership(uavImage, *pComputeQueue));

        // No need for an image barrier, since the layout does not change, only a queue ownership ocurred!
        ClearUnorderedAccessViewInfo clearUAVInfo = { .view = uav, .clearValue = { 1, 0, 1, 1 } };
        TRACK_RHI_PARAMETER(clearUAVInfo);
        EXPECT_NO_THROW(graphicsCommands->ClearUnorderedAccessView(clearUAVInfo));

        graphicsCommands->Complete();
        graphicsSubmitInfo.commands = { &graphicsCommands, 1 };
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
    submitInfo.commands = { &commandBuffer, 1 };
    // Submit 1
    mDevice->SubmitQueue(submitInfo);
    // Wait
    submitInfo.queue->WaitIdle();
    // Get new commands
    commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "test2 cmds" });
    TRACK_RHI_HANDLE(commandBuffer);
    commandBuffer->Complete();
    submitInfo.commands = { &commandBuffer, 1 };
    // Submit 2
    mDevice->SubmitQueue(submitInfo);
    // Finally wait for queue to finish.
    submitInfo.queue->WaitIdle();
}

// !!Potential flaky test!!
TEST_F(RHI_CONTEXT_FIXTURE_NAME, SingleQueueDestroyDeferredSuccess) {
    // repeat a couple times to make sure this wasn't a fluke!
    static constexpr i32 NUM_TEST_REPEATS = 4;
    for (i32 x__ = 0; x__ < NUM_TEST_REPEATS; ++x__) {
        static constexpr i32 NUM_DESTROY_DEFERRED_CYCLES = 16;

        ICommandQueue* cq = mDevice->GetCommandQueues()[0];
        TRACK_RHI_HANDLE(cq);

        eastl::vector<ICommandBuffer*> commands = {};

        for (i32 i = 0; i < NUM_DESTROY_DEFERRED_CYCLES; ++i) {
            commands.emplace_back(cq->GetCommandBuffer({ .name = "destroy deferred commands #" + eastl::to_string(i) }));
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

            ICommandBuffer* cb = commands[i];
            TRACK_RHI_HANDLE(cb);

            // It should still be legal to use these resources after destroying! They are still valid in this frame!
            mDevice->DestroyDeferred(srcImage);
            mDevice->DestroyDeferred(dstBuffer);
            mDevice->DestroyDeferred(block);

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
            mDevice->SubmitQueue({ .queue = cq, .commands = { &cb, 1 } });

            // Clean up anything ready for destruction
            mDevice->CollectGarbage();
        }
        mDevice->WaitIdle();
    }
}

// !!Potential flaky test!!
TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiQueueDestroyDeferredSuccess) {
    // repeat a couple times to make sure this wasn't a fluke!
    static constexpr i32 NUM_TEST_REPEATS = 4;
    for (i32 x__ = 0; x__ < NUM_TEST_REPEATS; ++x__) {
        static constexpr i32 NUM_DESTROY_DEFERRED_CYCLES = 16;

        auto queues = mDevice->GetCommandQueues();
        for (ICommandQueue* queue : queues) {
            TRACK_RHI_HANDLE(queue);
        }
        ASSERT_FALSE(queues.empty());

        ICommandQueue** pTransferQueue1 = eastl::find_if(queues.begin(), queues.end(),
            [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER; });
        ASSERT_NE(pTransferQueue1, nullptr);
        TRACK_RHI_HANDLE(*pTransferQueue1);
        ICommandQueue** pTransferQueue2 = eastl::find_if(queues.begin(), queues.end(),
            [pTransferQueue1](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER && q != *pTransferQueue1; });
        if (pTransferQueue2 == queues.end()) {
            GTEST_SKIP() << "Device does not support multiple queues, skipping test...";
        }


        ICommandQueue* tq1 = *pTransferQueue1;
        ICommandQueue* tq2 = *pTransferQueue2;

        eastl::vector<ICommandBuffer*> commandsQueue1 = {};
        eastl::vector<ICommandBuffer*> commandsQueue2 = {};

        eastl::vector<Semaphore> signalT2Semaphores = {};

        for (i32 i = 0; i < NUM_DESTROY_DEFERRED_CYCLES; ++i) {
            commandsQueue1.emplace_back(tq1->GetCommandBuffer({ .name = "ddc transferQueue1 #" + eastl::to_string(i) }));
            commandsQueue2.emplace_back(tq2->GetCommandBuffer({ .name = "ddc transferQueue1 #" + eastl::to_string(i) }));
            signalT2Semaphores.emplace_back(mDevice->CreateSemaphore({ .name = "waitForT1Semaphore #" + eastl::to_string(i) }));
        }

        for (i32 i = 0; i < NUM_DESTROY_DEFERRED_CYCLES; ++i) {
            ICommandBuffer* cb1 = commandsQueue1[i];
            TRACK_RHI_HANDLE(cb1);
            ICommandBuffer* cb2 = commandsQueue2[i];
            TRACK_RHI_HANDLE(cb2);

            BufferInfo bufferInfo{};
            bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
            bufferInfo.name = "dst buffer #" + eastl::to_string(i);
            bufferInfo.size = 256;
            TRACK_RHI_PARAMETER(bufferInfo);
            Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
            TRACK_RHI_HANDLE(dstBuffer);
            ASSERT_TRUE(mDevice->IsValid(dstBuffer));

            // no matter when this is called, this should work
            mDevice->DestroyDeferred(dstBuffer);
            // Interesting functionality, this should be safe to call, since this should only trigger destruction once the second queue is finished executing,
            // which is *after* the semaphore was used.
            mDevice->DestroyDeferred(signalT2Semaphores[i]);
            eastl::vector<u8> data(static_cast<usize>(bufferInfo.size));

            SemaphoreSubmitInfo currFrameSemaphoreSubmit{
                .semaphore = signalT2Semaphores[i],
                .stage = PipelineStageFlagBits::BOTTOM_OF_PIPE | PipelineStageFlagBits::TRANSFER,
            };
            // Use buffer in Queue 1
            {
                eastl::string framename1 = "Record frame TQ1 #" + eastl::to_string(i);
                eastl::string framename = "Transition Resources TQ1 #" + eastl::to_string(i);
                eastl::string framenameAqc = "Release Resources TQ1 #" + eastl::to_string(i);
                cb1->BeginLabel({ .name = framename1 });
                cb1->BeginLabel({ .name = framename });
                EXPECT_NO_FATAL_FAILURE(cb1->BufferBarrier({
                    .buffer = dstBuffer,
                    .srcAccess = AccessConsts::NONE,
                    .dstAccess = AccessConsts::TRANSFER_WRITE,
                    .srcLayout = BufferLayout::Undefined,
                    .dstLayout = BufferLayout::TransferDst,
                }));
                cb1->EndLabel();

                UpdateBufferInfo info{};
                info.buffer = dstBuffer;
                info.data = data.data();
                TRACK_RHI_PARAMETER(info);

                EXPECT_NO_FATAL_FAILURE(cb1->UpdateBuffer(info));

                cb1->BeginLabel({ .name = framename });
                EXPECT_NO_FATAL_FAILURE(cb1->TransferBufferOwnership(dstBuffer, tq2));
                cb1->EndLabel();

                cb1->EndLabel();

                cb1->Complete();
                mDevice->SubmitQueue({ .queue = tq1, .commands = { &cb1, 1 }, .signalSemaphores = { &currFrameSemaphoreSubmit, 1 } });
            }

            // Use buffer in Queue 2
            {
                eastl::string framename1 = "Record frame TQ2 #" + eastl::to_string(i);
                eastl::string framenameAqc = "Acquire Resources TQ2 #" + eastl::to_string(i);
                cb2->BeginLabel({ .name = framename1 });

                cb2->BeginLabel({ .name = framenameAqc });
                EXPECT_NO_FATAL_FAILURE(cb2->AcquireBufferOwnership(dstBuffer, tq1));
                cb2->EndLabel();

                UpdateBufferInfo info{};
                info.buffer = dstBuffer;
                info.data = data.data();
                TRACK_RHI_PARAMETER(info);

                EXPECT_NO_FATAL_FAILURE(cb2->UpdateBuffer(info));
                cb2->EndLabel();

                cb2->Complete();
                mDevice->SubmitQueue({ .queue = tq2, .commands = { &cb2, 1 }, .waitSemaphores = { &currFrameSemaphoreSubmit, 1 } });
            }
            // Clean up anything ready for destruction
            mDevice->CollectGarbage();
        }
        // finally, wait for everything to clean up
        mDevice->WaitIdle();
    }
}
TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiQueueResourceTransferSuccess) {
    auto queues = mDevice->GetCommandQueues();
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());

    ICommandQueue** pTransferQueue1 = eastl::find_if(queues.begin(), queues.end(),
        [](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER; });
    ASSERT_NE(pTransferQueue1, nullptr);
    TRACK_RHI_HANDLE(*pTransferQueue1);
    ICommandQueue** pTransferQueue2 = eastl::find_if(queues.begin(), queues.end(),
        [pTransferQueue1](ICommandQueue* q) { return q->Info().flags & CommandQueueFlagBits::TRANSFER && q != *pTransferQueue1; });
    if (pTransferQueue2 == queues.end()) {
        GTEST_SKIP() << "Device does not support multiple queues, skipping test...";
    }


    ICommandQueue* tq1 = *pTransferQueue1;
    ICommandQueue* tq2 = *pTransferQueue2;


    ICommandBuffer* cb1 = tq1->GetCommandBuffer({ .name = "Cb Queue 1" });
    TRACK_RHI_HANDLE(cb1);
    ICommandBuffer* cb2 = tq2->GetCommandBuffer({ .name = "Cb Queue 2" });
    TRACK_RHI_HANDLE(cb2);


    ImageInfo srcImageInfo{};
    srcImageInfo.dimensions = ImageDimensions::e3D;
    srcImageInfo.size = { 16, 16, 4 };
    srcImageInfo.format = Format::RGBA8Unorm;
    srcImageInfo.usage = ImageUsageFlagBits::TRANSFER_SRC;
    srcImageInfo.name = "src image";
    TRACK_RHI_PARAMETER(srcImageInfo);
    Image srcImage = mDevice->CreateImage(srcImageInfo);
    TRACK_RHI_HANDLE(srcImage);
    ASSERT_TRUE(mDevice->IsValid(srcImage));

    BufferInfo bufferInfo{};
    bufferInfo.initialLayout = BufferLayout::TransferDst;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.name = "dst buffer";

    CopyImageToBufferInfo copyInfo{};
    copyInfo.image = srcImage;
    copyInfo.imageExtent = srcImageInfo.size;
    bufferInfo.size = mDevice->ImageSizeRequirements(srcImage);
    copyInfo.rowPitch = bufferInfo.size / srcImageInfo.size.height / srcImageInfo.size.depth;
    copyInfo.rowPitch = mDevice->ImageSubresourceRowPitch(srcImage, copyInfo.rowPitch);

    bufferInfo.size = srcImageInfo.size.height * srcImageInfo.size.depth * copyInfo.rowPitch;
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(dstBuffer);
    ASSERT_TRUE(mDevice->IsValid(dstBuffer));

    copyInfo.buffer = dstBuffer;
    TRACK_RHI_PARAMETER(copyInfo);

    // Use buffer & image in Queue 1
    {
        cb1->BeginLabel({ .name = "Queue 1 usage" });
        cb1->BeginLabel({ .name = "Init" });
        EXPECT_NO_FATAL_FAILURE(cb1->ImageBarrier({
            .image = srcImage,
            .srcAccess = AccessConsts::NONE,
            .dstAccess = AccessConsts::TRANSFER_READ,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::TransferSrc,
        }));
        cb1->EndLabel();

        EXPECT_NO_FATAL_FAILURE(cb1->CopyImageToBuffer(copyInfo));

        cb1->BeginLabel({ .name = "Ownership Transfer" });
        EXPECT_NO_FATAL_FAILURE(cb1->TransferImageOwnership(srcImage, tq2));
        EXPECT_NO_FATAL_FAILURE(cb1->TransferBufferOwnership(dstBuffer, tq2));
        cb1->EndLabel();

        cb1->EndLabel();

        cb1->Complete();
        mDevice->SubmitQueue({ .queue = tq1, .commands = { &cb1, 1 } });
    }
    tq1->WaitIdle(); // wait
    // Use buffer & image in Queue 2
    {
        cb2->BeginLabel({ .name = "Queue 2 usage" });
        cb2->BeginLabel({ .name = "Ownership Transfer" });
        EXPECT_NO_FATAL_FAILURE(cb2->AcquireImageOwnership(srcImage, tq1));
        EXPECT_NO_FATAL_FAILURE(cb2->AcquireBufferOwnership(dstBuffer, tq1));
        cb2->EndLabel();

        EXPECT_NO_FATAL_FAILURE(cb2->CopyImageToBuffer(copyInfo));

        cb2->EndLabel();

        cb2->Complete();
        mDevice->SubmitQueue({ .queue = tq2, .commands = { &cb2, 1 } });
    }
    mDevice->WaitIdle();
    mDevice->DestroyImmediately(srcImage);
    mDevice->DestroyImmediately(dstBuffer);
}


// Flaky test!
TEST_F(RHI_CONTEXT_FIXTURE_NAME, ConcurrentQueueSubmits) {
    static constexpr int NUM_SUBMITS = 100;
    static constexpr int NUM_WORKERS = 4;
    std::vector<std::jthread> workers = {};

    auto queues = mDevice->GetCommandQueues();
    for (ICommandQueue* queue : queues) {
        TRACK_RHI_HANDLE(queue);
    }
    ASSERT_FALSE(queues.empty());
    TRACK_RHI_HANDLE(queues[0]);

    std::shared_mutex startLock;
    auto workerFun = [&](int workerIndex) {
        std::shared_lock lk(startLock); // waiting for unique lock to be released, then all threads start simultaneously

        CommandQueueSubmitInfo submitInfo = {};
        submitInfo.queue = queues[0];
        for (int i = 0; i < NUM_SUBMITS; ++i) {
            ICommandBuffer* commandBuffer = submitInfo.queue->GetCommandBuffer({ .name = "thread #" + eastl::to_string(workerIndex) + " cmds | submit #" + eastl::to_string(i) });
            commandBuffer->Complete();
            submitInfo.commands = { &commandBuffer, 1 };
            mDevice->SubmitQueue(submitInfo);
        }
    };
    {
        std::unique_lock lk(startLock);
        for (int i = 0; i < NUM_WORKERS; ++i) {
            workers.push_back(std::jthread(workerFun, i));
        }
    } // startLock is released, now all threads can start
    for (auto& job : workers) {
        if (job.joinable()) {
            job.join();
        }
    }
}
TEST_F(RHI_CONTEXT_FIXTURE_NAME, DestroyDeferredOpenCommandListDenied) {
    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());

    ICommandQueue* queue = queues[0];
    TRACK_RHI_HANDLE(queue);

    // Create a dummy buffer
    BufferInfo bufferInfo{};
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
    bufferInfo.size = 256;
    bufferInfo.name = "Open Cmd List Test Buffer";
    TRACK_RHI_PARAMETER(bufferInfo);

    Buffer dstBuffer = mDevice->CreateBuffer(bufferInfo);
    TRACK_RHI_HANDLE(dstBuffer);
    ASSERT_TRUE(mDevice->IsValid(dstBuffer));

    // Open a command buffer but DO NOT complete or submit it yet
    ICommandBuffer* cb = queue->GetCommandBuffer({ .name = "Unsubmitted Commands" });
    TRACK_RHI_HANDLE(cb);

    // Record a command to ensure the resource is utilized
    EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
        .buffer = dstBuffer,
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .srcLayout = BufferLayout::Undefined,
        .dstLayout = BufferLayout::TransferDst,
    }));

    // Mark the resource for deferred destruction
    mDevice->DestroyDeferred(dstBuffer);

    // Attempt to collect garbage. This should return false as there are open command lists!
    // Because 'cb' is still open/unsubmitted, your new feature should prevent destruction.
    ASSERT_FALSE(mDevice->CollectGarbage());

    // Continue using the resource.
    // If CollectGarbage() prematurely destroyed it, the validation layer will catch it here or on submit.
    eastl::vector<u8> dummyData(static_cast<usize>(bufferInfo.size), 0xAA);
    UpdateBufferInfo updateInfo{};
    updateInfo.buffer = dstBuffer;
    updateInfo.data = dummyData.data();
    TRACK_RHI_PARAMETER(updateInfo);

    EXPECT_NO_FATAL_FAILURE(cb->UpdateBuffer(updateInfo));

    // Complete and submit
    cb->Complete();
    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queue;
    submitInfo.commands = { &cb, 1 };
    mDevice->SubmitQueue(submitInfo);

    // Now that the queue is idle and there are no open command lists,
    // this final CollectGarbage should safely and legally be able to determine whether or not to destroy the resource.
    ASSERT_TRUE(mDevice->CollectGarbage());
}

// flaky test!
TEST_F(RHI_CONTEXT_FIXTURE_NAME, ConcurrentDestroyDeferredStressTest) {
    static constexpr int NUM_WORKERS = 6;
    static constexpr int NUM_SUBMITS_PER_WORKER = 100;

    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());
    ICommandQueue* mainQueue = queues[0];
    TRACK_RHI_HANDLE(mainQueue);

    std::vector<std::jthread> workers;
    std::shared_mutex startLock;
    eastl::atomic<int> completedWorkers = 0;

    auto workerFun = [&](int workerIndex) {
        std::shared_lock lk(startLock); // Wait until the main thread releases the lock

        for (int i = 0; i < NUM_SUBMITS_PER_WORKER; ++i) {
            // Create a local resource for this specific submission
            BufferInfo bufferInfo{};
            bufferInfo.usage = BufferUsageFlagBits::TRANSFER_DST;
            bufferInfo.size = 256;
            bufferInfo.name = "Stress Buffer W" + eastl::to_string(workerIndex) + " I" + eastl::to_string(i);
            Buffer tempBuffer = mDevice->CreateBuffer(bufferInfo);

            // Open a local command buffer
            ICommandBuffer* cb = mainQueue->GetCommandBuffer({ .name = "Stress Cmd W" + eastl::to_string(workerIndex) + " I" + eastl::to_string(i) });

            // Record commands to ensure the resource is in use
            EXPECT_NO_FATAL_FAILURE(cb->BufferBarrier({
                .buffer = tempBuffer,
                .srcAccess = AccessConsts::NONE,
                .dstAccess = AccessConsts::TRANSFER_WRITE,
                .srcLayout = BufferLayout::Undefined,
                .dstLayout = BufferLayout::TransferDst,
            }));

            // Mark for deferred destruction BEFORE submitting.
            // This tests if the garbage collector correctly identifies that the resource
            // is tied to a timeline value that hasn't finished (or hasn't even been submitted) yet.
            mDevice->DestroyDeferred(tempBuffer);

            // Complete and submit safely
            cb->Complete();
            CommandQueueSubmitInfo submitInfo = {};
            submitInfo.queue = mainQueue;
            submitInfo.commands = { &cb, 1 };

            mDevice->SubmitQueue(submitInfo);
        }
        completedWorkers.fetch_add(1);
    };

    // Spin up the threads
    {
        std::unique_lock lk(startLock);
        for (int i = 0; i < NUM_WORKERS; ++i) {
            workers.push_back(std::jthread(workerFun, i));
        }
    } // startLock released, the chaos begins

    // The Stress: Continually run garbage collection while threads are actively
    // appending zombies and pending submissions to the device lists.
    while (completedWorkers.load() < NUM_WORKERS) {
        mDevice->CollectGarbage(); // result may be true or false
        std::this_thread::yield();
    }

    // Wait for threads to fully exit
    for (auto& job : workers) {
        if (job.joinable()) {
            job.join();
        }
    }

    ASSERT_TRUE(mDevice->CollectGarbage());
}