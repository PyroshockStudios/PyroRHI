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

    ISwapChain* swapChain = mDevice->CreateSwapChain({
        .format = SwapChainFormat::Unorm8BitLDR,
        .bufferCount = 2,
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
        .srcAccess = AccessConsts::NONE,
        .dstAccess = AccessConsts::READ,
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
