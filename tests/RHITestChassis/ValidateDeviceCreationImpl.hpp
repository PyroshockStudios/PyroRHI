#include "Helpers/ValidationFixture.hpp"

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, ValidateDeviceCreation) {
    // const DeviceInfo& info = mDevice->GetInfo();
    // EXPECT_FALSE(info.name.empty());
    // EXPECT_FALSE(info.vendor.empty());
    // EXPECT_GT(info.driverVersion, 0u);

    const DevicePropertiesInfo& props = mDevice->GetProperties();
    EXPECT_GE(props.maxRenderTargetSamples, RasterizationSamples::e1);
    EXPECT_GE(props.bufferImageRowAlignment, 1u);
}
