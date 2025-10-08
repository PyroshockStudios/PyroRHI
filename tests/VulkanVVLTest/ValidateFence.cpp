#include "Helpers/ValidationFixture.hpp"
namespace PyroshockStudios::RHI::Tests {
    TEST_F(VulkanContextFixture, CreateAndDestroyFence) {
        FenceInfo fenInfo = {};
        fenInfo.name = "TestFence";
        fenInfo.initialValue = 0;

        IFence* fence = mDevice->CreateFence(fenInfo);
        mDevice->DestroyFence(fence);
    }
} // namespace PyroshockStudios::RHI::Tests
