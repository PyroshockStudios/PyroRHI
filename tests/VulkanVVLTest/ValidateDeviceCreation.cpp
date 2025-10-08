#include "Helpers/ValidationFixture.hpp"

namespace PyroshockStudios::RHI::Tests {
    TEST_F(VulkanContextFixture, ValidateDeviceCreation) {
        // const DeviceInfo& info = mDevice->GetInfo();
        // EXPECT_FALSE(info.name.empty());
        // EXPECT_FALSE(info.vendor.empty());
        // EXPECT_GT(info.driverVersion, 0u);

        const DevicePropertiesInfo& props = mDevice->GetProperties();
        EXPECT_GE(props.maxRenderTargetSamples, RasterizationSamples::e1);
        EXPECT_GE(props.bufferImageRowAlignment, 1u);
    }
} // namespace PyroshockStudios::RHI::Tests
