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
