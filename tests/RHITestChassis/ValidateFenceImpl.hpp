#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyFence) {
    FenceInfo fenInfo = {};
    fenInfo.name = "TestFence";
    fenInfo.initialValue = 0;

    IFence* fence = mDevice->CreateFence(fenInfo);
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

    EXPECT_NO_FATAL_FAILURE(bSuccess = fence->WaitForValue(FenceVal, 1ULL * 1000 * 1000 * 1000));
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
