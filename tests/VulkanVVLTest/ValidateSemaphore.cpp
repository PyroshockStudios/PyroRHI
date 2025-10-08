#pragma once

#include "Helpers/ValidationFixture.hpp"
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
namespace PyroshockStudios::RHI::Tests {
    TEST_F(VulkanContextFixture, CreateAndDestroySemaphore) {
        SemaphoreInfo semInfo = {};
        semInfo.name = "TestSemaphore";
        Semaphore semaphore = mDevice->CreateSemaphore(semInfo);
        mDevice->DestroySemaphore(semaphore);
    }
} // namespace PyroshockStudios::RHI::Tests
