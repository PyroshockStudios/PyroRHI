#define RHI_TEST_CHASSIS_API_LIB "RHIDX12"
#define RHI_TEST_CHASSIS_API_LOG_NAME "DirectX12"
#define RHI_TEST_CHASSIS_API_VALIDATOR_NAME "D3D12Debug"
#define RHI_CONTEXT_FIXTURE_NAME DX12ValidationChassis
#define RHI_IMPL_DX12 1

// clang-format off
#define RHI_TEST_CHASSIS_RHI_OPTIONS                                  \
/*"debug"*/         mCreateInfo.options[0] = { .optionIndex = 0 };    \
/*"warp driver"*/   /*mCreateInfo.options[1] = { .optionIndex = 1 };*/\
                    mCreateInfo.options[1] = { .optionIndex = -1 };
// clang-format on

//#include <RHITestChassis/ValidatePipelineImpl.hpp>
//#include <RHITestChassis/ValidateAccelerationStructureImpl.hpp>
//#include <RHITestChassis/ValidateBufferImpl.hpp>
#include <RHITestChassis/ValidateCommandBufferImpl.hpp>
//#include <RHITestChassis/ValidateCommandQueueImpl.hpp>
//#include <RHITestChassis/ValidateDeviceCreationImpl.hpp>
//#include <RHITestChassis/ValidateFenceImpl.hpp>
//#include <RHITestChassis/ValidateImageImpl.hpp>
//#include <RHITestChassis/ValidateQueryPoolImpl.hpp>
//#include <RHITestChassis/ValidateSemaphoreImpl.hpp>
//#include <RHITestChassis/ValidateSwapChainImpl.hpp>
