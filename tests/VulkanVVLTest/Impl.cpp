#define RHI_TEST_CHASSIS_API_LIB "RHIVulkan"
#define RHI_TEST_CHASSIS_API_LOG_NAME "VULKAN"
#define RHI_TEST_CHASSIS_API_VALIDATOR_NAME "VVL"
#define RHI_CONTEXT_FIXTURE_NAME VulkanValidationChassis
// clang-format off
#define RHI_TEST_CHASSIS_RHI_OPTIONS                                \
/*"debug"*/         mCreateInfo.options[0] = { .optionIndex = 0 };  \
/*"headless"*/      mCreateInfo.options[1] = { .optionIndex = 1 };  \
                    mCreateInfo.options[2] = { .optionIndex = -1 };
// clang-format on


#include <RHITestChassis/ValidateBufferImpl.hpp>
#include <RHITestChassis/ValidateCommandBufferImpl.hpp>
#include <RHITestChassis/ValidateCommandQueueImpl.hpp>
#include <RHITestChassis/ValidateDeviceCreationImpl.hpp>
#include <RHITestChassis/ValidateFenceImpl.hpp>
#include <RHITestChassis/ValidateImageImpl.hpp>
#include <RHITestChassis/ValidateSemaphoreImpl.hpp>
