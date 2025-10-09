#include "Helpers/ValidationFixture.hpp"
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
