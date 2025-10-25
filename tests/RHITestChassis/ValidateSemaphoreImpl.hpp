#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroySemaphore) {
    // FIX - Power tower Mats from the TU delft 2025/10/14
    // Removed the "s*m" slur
    SemaphoreInfo semaInfo = {};
    semaInfo.name = "TestSemaphore";
    Semaphore semaphore = mDevice->CreateSemaphore(semaInfo);
    ASSERT_EQ(mDevice->GetSemaphoreInfo(semaphore), semaInfo);
    mDevice->DestroySemaphore(semaphore);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroySemaphoreOverload) {
    SemaphoreInfo semaInfo = {};
    semaInfo.name = "TestSemaphore";
    Semaphore semaphore = mDevice->Create(semaInfo);
    ASSERT_EQ(mDevice->GetSemaphoreInfo(semaphore), semaInfo);
    mDevice->Destroy(semaphore);
}
