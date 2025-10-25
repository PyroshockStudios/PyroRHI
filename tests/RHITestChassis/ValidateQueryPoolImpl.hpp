#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyTimestampQueryPool) {
    TimestampQueryPoolInfo info = { .queryCount = 2, .name = " qp" };
    ITimestampQueryPool* qp = mDevice->CreateTimestampQueryPool(info);
    ASSERT_NE(qp, nullptr);
    ASSERT_EQ(info, qp->Info());
    mDevice->DestroyTimestampQueryPool(qp);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyTimestampQueryPoolOverload) {
    TimestampQueryPoolInfo info = { .queryCount = 2, .name = " qp" };
    ITimestampQueryPool* qp = mDevice->Create(info);
    ASSERT_NE(qp, nullptr);
    ASSERT_EQ(info, qp->Info());
    mDevice->Destroy(qp);
}
