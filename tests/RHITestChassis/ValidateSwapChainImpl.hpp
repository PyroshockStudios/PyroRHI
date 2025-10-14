#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

static constexpr u32 SWAP_ACQUIRE_FAIL_LIMIT = 4; // after 4 fails, fail the test

TEST_F(RHI_CONTEXT_FIXTURE_NAME, SwapPresentSuccess) {
    if (!mDevice->GetProperties().bSupportsHeadlessSwapChainWindow) {
        GTEST_LOG_(INFO) << "Device does not support a headless swap chain, skipping test...";
        return;
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    Semaphore waitPresentSemaphore = mDevice->CreateSemaphore({ .name = "waitPresentSemaphore" });

    SwapChainInfo info = {
        .format = SwapChainFormat::Unorm8BitLDR,
        .bufferCount = 2,
        .imageUsage =
            ImageUsageFlagBits::TRANSFER_SRC |
            ImageUsageFlagBits::TRANSFER_DST |
            ImageUsageFlagBits::SHADER_RESOURCE |
            ImageUsageFlagBits::UNORDERED_ACCESS |
            ImageUsageFlagBits::RENDER_TARGET |
            ImageUsageFlagBits::BLIT_SRC |
            ImageUsageFlagBits::BLIT_DST, // test many usages
        .extent = { 256, 256 },
        .name = "Headless Swap Chain",
    };

    ISwapChain* swapChain = mDevice->CreateSwapChain(info);
    ASSERT_EQ(info.alphaMode, swapChain->Info().alphaMode);
    ASSERT_EQ(info.extent, swapChain->Info().extent);
    ASSERT_EQ(info.format, swapChain->Info().format);
    ASSERT_EQ(info.imageUsage, swapChain->Info().imageUsage);
    ASSERT_EQ(info.name, swapChain->Info().name);

    ASSERT_EQ(info.name, swapChain->GetSurfaceExtent());

    const SemaphoreSubmitInfo signalPresent{
        .semaphore = waitPresentSemaphore,
        .stage = PipelineStageFlagBits::ALL_COMMANDS
    };

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queue;
    submitInfo.signalPresentReadySemaphores = eastl::span(&signalPresent, 1);

    CommandQueuePresentInfo presentInfo = {};
    presentInfo.queue = queue;
    presentInfo.waitSemaphores = eastl::span(&waitPresentSemaphore, 1);

    Image image = PYRO_NULL_IMAGE;
    u32 failedAcquires = 0;
    do {
        image = swapChain->AcquireNextImage();
    } while (image == PYRO_NULL_IMAGE && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
    ASSERT_NE(image, PYRO_NULL_IMAGE) << "Failed to acquire swap image!";

    ICommandBuffer* commandBuffer = queue->GetCommandBuffer({});
    commandBuffer->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::BOTTOM_OF_PIPE_READ,
        .dstAccess = AccessConsts::TOP_OF_PIPE_READ_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::PresentSrc,
    });
    commandBuffer->Complete();
    queue->SubmitCommandBuffer(commandBuffer);
    queue->SubmitSwapChain(swapChain);

    mDevice->SubmitQueue(submitInfo);
    mDevice->PresentQueue(presentInfo);
    mDevice->WaitIdle();
    mDevice->Destroy(waitPresentSemaphore);
    mDevice->Destroy(swapChain);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, SwapAlphaPresentSuccess) {
    if (!mDevice->GetProperties().bSupportsHeadlessSwapChainWindow) {
        GTEST_LOG_(INFO) << "Device does not support a headless swap chain, skipping test...";
        return;
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    Semaphore waitPresentSemaphore = mDevice->CreateSemaphore({ .name = "waitPresentSemaphore" });

    ISwapChain* swapChain = mDevice->CreateSwapChain({
        .format = SwapChainFormat::Unorm8BitLDR,
        .alphaMode = SwapChainAlphaMode::Premultiplied,
        .presentMode = PresentMode::Tearing, // try another present mode
        .bufferCount = 2,
        .imageUsage = ImageUsageFlagBits::TRANSFER_DST, // we need at least 1 image usage
        .extent = { 256, 256 },
        .name = "Headless Swap Chain",
    });

    const SemaphoreSubmitInfo signalPresent{
        .semaphore = waitPresentSemaphore,
        .stage = PipelineStageFlagBits::ALL_COMMANDS
    };

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queue;
    submitInfo.signalPresentReadySemaphores = eastl::span(&signalPresent, 1);

    CommandQueuePresentInfo presentInfo = {};
    presentInfo.queue = queue;
    presentInfo.waitSemaphores = eastl::span(&waitPresentSemaphore, 1);

    Image image = PYRO_NULL_IMAGE;
    u32 failedAcquires = 0;
    do {
        image = swapChain->AcquireNextImage();
    } while (image == PYRO_NULL_IMAGE && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
    ASSERT_NE(image, PYRO_NULL_IMAGE) << "Failed to acquire swap image!";

    ICommandBuffer* commandBuffer = queue->GetCommandBuffer({});
    commandBuffer->ImageBarrier({
        .image = image,
        .srcAccess = AccessConsts::BOTTOM_OF_PIPE_READ,
        .dstAccess = AccessConsts::TOP_OF_PIPE_READ_WRITE,
        .srcLayout = ImageLayout::Undefined,
        .dstLayout = ImageLayout::PresentSrc,
    });
    commandBuffer->Complete();
    queue->SubmitCommandBuffer(commandBuffer);
    queue->SubmitSwapChain(swapChain);

    mDevice->SubmitQueue(submitInfo);
    mDevice->PresentQueue(presentInfo);
    mDevice->WaitIdle();
    mDevice->Destroy(waitPresentSemaphore);
    mDevice->Destroy(swapChain);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiSwapPresentSuccess) {
    static constexpr i32 NUM_SWAPCHAINS = 8;

    if (!mDevice->GetProperties().bSupportsHeadlessSwapChainWindow) {
        GTEST_LOG_(INFO) << "Device does not support a headless swap chain, skipping test...";
        return;
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    Semaphore waitPresentSemaphore = mDevice->CreateSemaphore({ .name = "waitPresentSemaphore" });

    eastl::vector<ISwapChain*> swapChains = {};
    for (i32 i = 0; i < NUM_SWAPCHAINS; ++i) {
        swapChains.push_back(mDevice->CreateSwapChain({
            .format = SwapChainFormat::Unorm8BitLDR,
            .bufferCount = 2,
            .imageUsage = ImageUsageFlagBits::TRANSFER_DST, // we need at least 1 image usage
            .extent = { 256, 256 },
            .name = "Headless Swap Chain #" + eastl::to_string(i),
        }));
    }
    const SemaphoreSubmitInfo signalPresent{
        .semaphore = waitPresentSemaphore,
        .stage = PipelineStageFlagBits::ALL_COMMANDS
    };

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = queue;
    submitInfo.signalPresentReadySemaphores = eastl::span(&signalPresent, 1);

    CommandQueuePresentInfo presentInfo = {};
    presentInfo.queue = queue;
    presentInfo.waitSemaphores = eastl::span(&waitPresentSemaphore, 1);
    ICommandBuffer* commandBuffer = queue->GetCommandBuffer({});

    for (ISwapChain* swapChain : swapChains) {
        Image image = PYRO_NULL_IMAGE;
        u32 failedAcquires = 0;
        do {
            image = swapChain->AcquireNextImage();
        } while (image == PYRO_NULL_IMAGE && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
        ASSERT_NE(image, PYRO_NULL_IMAGE) << "Failed to acquire swap image!";
        commandBuffer->ImageBarrier({
            .image = image,
            .srcAccess = AccessConsts::BOTTOM_OF_PIPE_READ,
            .dstAccess = AccessConsts::TOP_OF_PIPE_READ_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::PresentSrc,
        });
        queue->SubmitSwapChain(swapChain);
    }
    queue->SubmitCommandBuffer(commandBuffer);
    commandBuffer->Complete();

    mDevice->SubmitQueue(submitInfo);
    mDevice->PresentQueue(presentInfo);
    mDevice->WaitIdle();
    mDevice->Destroy(waitPresentSemaphore);
    for (ISwapChain* swapChain : swapChains) {
        mDevice->Destroy(swapChain);
    }
}
