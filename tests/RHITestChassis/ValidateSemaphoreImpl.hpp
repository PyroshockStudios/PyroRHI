#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroySemaphore) {
    SemaphoreInfo semInfo = {};
    semInfo.name = "TestSemaphore";
    Semaphore semaphore = mDevice->CreateSemaphore(semInfo);
    mDevice->DestroySemaphore(semaphore);
}
