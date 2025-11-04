#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

static constexpr u32 SWAP_ACQUIRE_FAIL_LIMIT = 4; // after 4 fails, fail the test


TEST_F(RHI_CONTEXT_FIXTURE_NAME, SwapChainCreateDestroyOverload) {
    if (!mDevice->Features().bHeadlessSwapChainWindow) {
        GTEST_SKIP() << "Device does not support a headless swap chain, skipping test...";
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    SwapChainInfo info = {
        .format = SwapChainFormat::Unorm8BitLDR,
        .bufferCount = 2,
        .imageUsage = ImageUsageFlagBits::TRANSFER_DST,
        .extent = { 256, 256 },
        .name = "Headless Swap Chain",
    };

    ISwapChain* swapChain = mDevice->Create(info);
    ASSERT_EQ(info.alphaMode, swapChain->Info().alphaMode);
    ASSERT_EQ(info.extent, swapChain->Info().extent);
    ASSERT_EQ(info.format, swapChain->Info().format);
    ASSERT_EQ(info.imageUsage, swapChain->Info().imageUsage);
    ASSERT_EQ(info.name, swapChain->Info().name);

    ASSERT_EQ(info.extent, swapChain->GetSurfaceExtent());

    mDevice->Destroy(swapChain);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, SwapPresentSuccess) {
    if (!mDevice->Features().bHeadlessSwapChainWindow) {
        GTEST_SKIP() << "Device does not support a headless swap chain, skipping test...";
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    Semaphore waitPresentSemaphore = mDevice->CreateSemaphore({ .name = "waitPresentSemaphore" });

    SwapChainInfo info = {
        .format = SwapChainFormat::Unorm8BitLDR,
        .bufferCount = 2,
        .imageUsage = ImageUsageFlagBits::TRANSFER_DST,
        .extent = { 256, 256 },
        .name = "Headless Swap Chain",
    };

    ISwapChain* swapChain = mDevice->CreateSwapChain(info);

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

    i32 imageIndex = -1;
    u32 failedAcquires = 0;
    do {
        imageIndex = swapChain->AcquireNextImage();
    } while (imageIndex == PYRO_SWAPCHAIN_ACQUIRE_FAIL && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
    ASSERT_NE(imageIndex, PYRO_SWAPCHAIN_ACQUIRE_FAIL) << "Failed to acquire swap image!";

    ICommandBuffer* commandBuffer = queue->GetCommandBuffer({});
    commandBuffer->ImageBarrier({
        .image = swapChain->GetBackBuffer(imageIndex),
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
    mDevice->DestroySemaphore(waitPresentSemaphore);
    mDevice->DestroySwapChain(swapChain);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, SwapAlphaPresentSuccess) {
    if (!mDevice->Features().bHeadlessSwapChainWindow) {
        GTEST_SKIP() << "Device does not support a headless swap chain, skipping test...";
    }
    ICommandQueue* queue = mDevice->GetPresentQueue();
    ASSERT_NE(queue, nullptr);

    Semaphore waitPresentSemaphore = mDevice->CreateSemaphore({ .name = "waitPresentSemaphore" });

    ISwapChain* swapChain = mDevice->CreateSwapChain({
        .format = SwapChainFormat::Unorm8BitLDR,
        .alphaMode = SwapChainAlphaMode::Premultiplied,
        .presentMode = SwapChainPresentMode::Tearing, // try another present mode
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

    i32 imageIndex = -1;
    u32 failedAcquires = 0;
    do {
        imageIndex = swapChain->AcquireNextImage();
    } while (imageIndex == PYRO_SWAPCHAIN_ACQUIRE_FAIL && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
    ASSERT_NE(imageIndex, PYRO_SWAPCHAIN_ACQUIRE_FAIL) << "Failed to acquire swap image!";

    ICommandBuffer* commandBuffer = queue->GetCommandBuffer({});
    commandBuffer->ImageBarrier({
        .image = swapChain->GetBackBuffer(imageIndex),
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
    mDevice->DestroySemaphore(waitPresentSemaphore);
    mDevice->DestroySwapChain(swapChain);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, MultiSwapPresentSuccess) {
    static constexpr i32 NUM_SWAPCHAINS = 8;

    if (!mDevice->Features().bHeadlessSwapChainWindow) {
        GTEST_SKIP() << "Device does not support a headless swap chain, skipping test...";
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
        i32 imageIndex = -1;
        u32 failedAcquires = 0;
        do {
            imageIndex = swapChain->AcquireNextImage();
        } while (imageIndex == PYRO_SWAPCHAIN_ACQUIRE_FAIL && ++failedAcquires < SWAP_ACQUIRE_FAIL_LIMIT);
        ASSERT_NE(imageIndex, PYRO_SWAPCHAIN_ACQUIRE_FAIL) << "Failed to acquire swap image!";
        commandBuffer->ImageBarrier({
            .image = swapChain->GetBackBuffer(imageIndex),
            .srcAccess = AccessConsts::BOTTOM_OF_PIPE_READ,
            .dstAccess = AccessConsts::TOP_OF_PIPE_READ_WRITE,
            .srcLayout = ImageLayout::Undefined,
            .dstLayout = ImageLayout::PresentSrc,
        });
        queue->SubmitSwapChain(swapChain);
    }
    commandBuffer->Complete();
    queue->SubmitCommandBuffer(commandBuffer);

    mDevice->SubmitQueue(submitInfo);
    mDevice->PresentQueue(presentInfo);
    mDevice->WaitIdle();
    mDevice->DestroySemaphore(waitPresentSemaphore);
    for (ISwapChain* swapChain : swapChains) {
        mDevice->DestroySwapChain(swapChain);
    }
}
