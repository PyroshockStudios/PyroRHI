#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;
TEST_F(RHI_CONTEXT_FIXTURE_NAME, EmptyQueueSubmitDoesNotError) {
    auto queues = mDevice->GetCommandQueues();
    ASSERT_FALSE(queues.empty());

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queues[0];
    ASSERT_NE(submitInfo.queue, nullptr);

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
    if (pCopyQueue == nullptr) {
        std::cout << "Device does not support seperate graphics and copy queues, ignoring...";
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

        EXPECT_NO_THROW(copyCommands->ImageBarrier({
            .image = src,
            .srcAccess = AccessConsts::TRANSFER_READ,
            .dstAccess = AccessConsts::BLIT_READ,
            .srcLayout = ImageLayout::TransferDst,
            .dstLayout = ImageLayout::BlitSrc,
            // Transfer queue ownership! Very important!
            .srcQueue = *pCopyQueue,
            .dstQueue = *pGraphicsQueue,
        }));

        copyCommands->Complete();
        (*pCopyQueue)->SubmitCommandBuffer(copyCommands);
    }
    // Record graphics commands
    {
        ICommandBuffer* graphicsCommands = (*pGraphicsQueue)->GetCommandBuffer({});

        BlitImageToImageInfo blitInfo{};
        blitInfo.srcImage = src;
        blitInfo.dstImage = dst;
        blitInfo.srcImageRect = Rect2D::Cut({ imageInfo.size.x, imageInfo.size.y });
        blitInfo.dstImageRect = Rect2D::Cut({ imageInfo.size.x, imageInfo.size.y });

        EXPECT_NO_THROW(graphicsCommands->ImageBarrier({
            .image = src,
            .srcAccess = AccessConsts::TRANSFER_READ,
            .dstAccess = AccessConsts::BLIT_READ,
            .srcLayout = ImageLayout::TransferDst,
            .dstLayout = ImageLayout::BlitSrc,
            // Transfer queue ownership! Very important!
            .srcQueue = *pCopyQueue,
            .dstQueue = *pGraphicsQueue,
        }));
        EXPECT_NO_THROW(graphicsCommands->ImageBarrier({
            .image = dst,
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
