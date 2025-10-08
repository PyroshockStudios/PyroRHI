#include "Helpers/ValidationFixture.hpp"
namespace PyroshockStudios::RHI::Tests {
    TEST_F(VulkanContextFixture, EmptyQueueSubmitDoesNotError) {
        auto queues = mDevice->GetCommandQueues();
        ASSERT_FALSE(queues.empty());

        CommandQueueSubmitInfo submitInfo = {};
        submitInfo.queue = queues[0];
        ASSERT_NE(submitInfo.queue, nullptr);

        mDevice->SubmitQueue(submitInfo);
        mDevice->WaitIdle();
    }
} // namespace PyroshockStudios::RHI::Tests
