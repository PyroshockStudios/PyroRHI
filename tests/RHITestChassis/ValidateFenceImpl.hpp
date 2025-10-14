#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

#include <thread>

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyFence) {
    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Info(), fenInfo);
    mDevice->DestroyFence(fence);
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

TEST_F(RHI_CONTEXT_FIXTURE_NAME, AsyncWaitBeforeTimeout) {
    static constexpr u64 FenceVal = 53;

    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
    ASSERT_EQ(fence->Value(), 0);

    auto asyncWait = std::thread([fence](){
        u64 fenceResultVal = 0;
        EXPECT_NO_FATAL_FAILURE(fenceResultVal = fence->WaitForValue(FenceVal, 5ULL * 1000'000'000));
    });
    asyncWait.detach();
    EXPECT_NO_FATAL_FAILURE(fence->SetValue(FenceVal));
    if (asyncWait.joinable()) {
        asyncWait.join();
    }
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

