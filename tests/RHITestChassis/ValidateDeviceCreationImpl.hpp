#include "Helpers/ValidationFixture.hpp"

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, ValidateDeviceCreation) {
    const DeviceInfo& info = mDevice->Info();
    EXPECT_FALSE(info.name.empty());
    EXPECT_FALSE(info.vendor.empty());
    EXPECT_FALSE(info.apiVersion.empty());
    EXPECT_FALSE(info.driverVersion.empty());

    const DevicePropertiesInfo& props = mDevice->Properties();
    EXPECT_GE(props.msaaSupportColorTarget, RasterizationSamples::e1);
    EXPECT_GE(props.msaaSupportDepthStencilTarget, RasterizationSamples::e1);
    EXPECT_GE(props.msaaSupportShaderResourceView, RasterizationSamples::e1);
    EXPECT_GE(props.msaaSupportUnorderedAccessView, RasterizationSamples::e1);
    EXPECT_GE(props.bufferImageRowAlignment, 1u);
    EXPECT_GE(props.bufferImageCopyOffsetAlignment, 1u);
    EXPECT_GE(props.minStorageBufferOffsetAlignment, 1u);
    EXPECT_GE(props.minUniformBufferOffsetAlignment, 1u);
    EXPECT_GT(props.minLineWidth, 0.0f);
    EXPECT_GE(props.maxLineWidth, props.minLineWidth);
    EXPECT_GE(props.maxSamplerAnisotropy, 1);
    EXPECT_GE(props.graphicsQueueCount, 1);
    EXPECT_GE(props.computeQueueCount, 1);
    EXPECT_GE(props.transferQueueCount, 1);
    EXPECT_GT(props.maxTextureWidth, 0);
    EXPECT_GT(props.maxTextureHeight, 0);
    EXPECT_GT(props.maxTextureDepth, 0);
    EXPECT_GT(props.maxTextureArrayLayers, 0);


    const DeviceFeaturesInfo& feats = mDevice->Features();
    EXPECT_GT(feats.maxSupportedShaderModel, 0);
}
