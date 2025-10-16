#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

#include <thread>
#include <latch>
#include <atomic>
#include <chrono>

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyFence) {
    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Info(), fenInfo);
    mDevice->DestroyFence(fence);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyFenceOverload) {
    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->Create(fenInfo);
    ASSERT_EQ(fence->Info(), fenInfo);
    mDevice->Destroy(fence);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, FenceSignalSuccess) {
    constexpr u64 FenceVal = 53;

    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Value(), 0);

    FenceSubmitInfo fenceSubmit{ .fence = fence, .value = FenceVal };

    CommandQueueSubmitInfo submitInfo = {};
    submitInfo.queue = mDevice->GetCommandQueues()[0];
    submitInfo.signalFences = eastl::span(&fenceSubmit, 1);
    mDevice->SubmitQueue(submitInfo);

    bool bSuccess = false;

    EXPECT_NO_FATAL_FAILURE(bSuccess = fence->WaitForValue(FenceVal, 1ULL * 1000'000'000));
    ASSERT_TRUE(bSuccess);
    ASSERT_EQ(fence->Value(), FenceVal);

    mDevice->DestroyFence(fence);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, FenceHostSignalSuccess) {
    constexpr u64 FenceVal = 53;

    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Value(), 0);

    EXPECT_NO_FATAL_FAILURE(fence->SetValue(FenceVal));
    ASSERT_EQ(fence->Value(), FenceVal);

    mDevice->DestroyFence(fence);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, AsyncWaitBeforeTimeout)
{
    static constexpr u64 FenceVal = 53;

    FenceInfo fenceInfo = {};
    fenceInfo.name = "TestFence";
    fenceInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenceInfo);
    ASSERT_EQ(fence->Value(), 0);

    std::latch waitStarted(1);
    std::atomic<bool> waitFinished = false;

    // Launch async wait thread with automatic join on destruction
    std::jthread asyncWait([&](std::stop_token st) {
        waitStarted.count_down(); // signal main thread that we’ve started waiting
        bool noTimeout = false;

        EXPECT_NO_FATAL_FAILURE(
            noTimeout = fence->WaitForValue(FenceVal, /*timeoutNs=*/5ULL * 1000'000'000)
        );
        ASSERT_TRUE(noTimeout) << "Fence timed out, even though value was set during timeout range";
        if (st.stop_requested()) {
            GTEST_SKIP() << "Test canceled before WaitForValue() finished.";
            return;
        }

        waitFinished = true;
        EXPECT_EQ(fence->Value(), FenceVal);
    });

    // Wait for the async thread to actually start waiting
    waitStarted.wait();

    // Simulate some work before signaling
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Signal the fence (should unblock WaitForValue)
    EXPECT_NO_FATAL_FAILURE(fence->SetValue(FenceVal));

    // Wait up to 2 seconds for it to finish
    using namespace std::chrono_literals;
    const auto start = std::chrono::steady_clock::now();
    while (!waitFinished && std::chrono::steady_clock::now() - start < 2s) {
        std::this_thread::sleep_for(10ms);
    }

    // If it didn't finish, request stop and fail the test
    if (!waitFinished) {
        asyncWait.request_stop();
        FAIL() << "WaitForValue() did not return within expected time after SetValue()";
    }

    // jthread automatically joins here
    mDevice->DestroyFence(fence);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, AsyncWaitAfterTimeout) {
    static constexpr u64 FenceVal = 53;
    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Value(), 0);

    bool result;
    EXPECT_NO_FATAL_FAILURE(result = fence->WaitForValue(FenceVal, 100ULL));
    EXPECT_FALSE(result); // timeout!
    mDevice->DestroyFence(fence);
}

